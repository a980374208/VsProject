#include "dxitem.h"
#include <QSGSimpleTextureNode>
#include <QQuickWindow>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSGTexture>
#include <QDebug>
#include "qtimer.h"

DXItem::DXItem()
    : m_device(nullptr),
    m_context(nullptr),
    m_swapChain(nullptr),
    m_renderTargetView(nullptr),
    m_renderTargetTexture(nullptr),
    m_texture(nullptr)
{
    setFlag(ItemHasContents, false);  // �����Զ�����
    connect(this, &QQuickItem::windowChanged, this, &DXItem::handleWindowChanged);
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [&]() {
        render();
       });
    timer->start(100);
}

DXItem::~DXItem()
{
    releaseResources();
}

void DXItem::releaseResources()
{
    if (m_renderTargetView) {
        m_renderTargetView->Release();
        m_renderTargetView = nullptr;
    }
    if (m_swapChain) {
        m_swapChain->Release();
        m_swapChain = nullptr;
    }
    if (m_context) {
        m_context->Release();
        m_context = nullptr;
    }
    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }
    if (m_renderTargetTexture) {
        m_renderTargetTexture->Release();
        m_renderTargetTexture = nullptr;
    }
    if (m_texture) {
        delete m_texture;
        m_texture = nullptr;
    }
}

void DXItem::handleWindowChanged(QQuickWindow* win)
{
    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &DXItem::initializeDirectX, Qt::DirectConnection);
        connect(win, &QQuickWindow::widthChanged, this, &DXItem::handleWindowSizeChanged);
        connect(win, &QQuickWindow::heightChanged, this, &DXItem::handleWindowSizeChanged);
        win->setClearBeforeRendering(false);
    }
}

void DXItem::handleWindowSizeChanged()
{
    if (m_swapChain) {
        m_context->OMSetRenderTargets(0, nullptr, nullptr);
        if (m_renderTargetView) {
            m_renderTargetView->Release();
            m_renderTargetView = nullptr;
        }
        HRESULT hr = m_swapChain->ResizeBuffers(0, width(), height(), DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr)) {
            qWarning() << "Failed to resize swap chain buffers";
            return;
        }

        ID3D11Texture2D* backBuffer;
        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        if (FAILED(hr)) {
            qWarning() << "Failed to get back buffer after resize";
            return;
        }
        hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
        backBuffer->Release();
        if (FAILED(hr)) {
            qWarning() << "Failed to create render target view after resize";
            return;
        }

        // ���´���������Ⱦ���������
        if (m_renderTargetTexture) {
            m_renderTargetTexture->Release();
            m_renderTargetTexture = nullptr;
        }
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = width();
        desc.Height = height();
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        hr = m_device->CreateTexture2D(&desc, nullptr, &m_renderTargetTexture);
        if (FAILED(hr)) {
            qWarning() << "Failed to create render target texture after resize";
            return;
        }

        if (m_texture) {
            delete m_texture;
            m_texture = nullptr;
        }

        update();  // ȷ������ˢ��
    }
}

void DXItem::initializeDirectX()
{
    if (!window()) return;

    if (m_device) return;  // DirectX �Ѿ���ʼ��

    // ��ʼ�� DirectX �豸��������
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        featureLevels,
        1,
        D3D11_SDK_VERSION,
        &m_device,
        &featureLevel,
        &m_context
    );

    if (FAILED(hr)) {
        qWarning() << "Failed to create D3D11 device";
        return;
    }

    // ��ȡ���ھ��
    HWND hwnd = (HWND)window()->winId();
    if (!IsWindow(hwnd)) {
        qWarning() << "Invalid window handle";
        return;
    }

    // ����������
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width();
    swapChainDesc.BufferDesc.Height = height();
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    IDXGIFactory* factory;
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(hr)) {
        qWarning() << "Failed to create DXGIFactory";
        return;
    }
    hr = factory->CreateSwapChain(m_device, &swapChainDesc, &m_swapChain);
    factory->Release();

    if (FAILED(hr)) {
        qWarning() << "Failed to create swap chain";
        return;
    }

    // ������ȾĿ����ͼ
    ID3D11Texture2D* backBuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr)) {
        qWarning() << "Failed to get back buffer";
        return;
    }
    hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
    backBuffer->Release();

    if (FAILED(hr)) {
        qWarning() << "Failed to create render target view";
        return;
    }

    // ����������Ⱦ���������
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width();
    desc.Height = height();
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = m_device->CreateTexture2D(&desc, nullptr, &m_renderTargetTexture);
    if (FAILED(hr)) {
        qWarning() << "Failed to create render target texture";
        return;
    }

    update();  // ȷ������ˢ��
}

QSGNode* DXItem::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData);

    if (!m_device) {
        initializeDirectX();
    }

    render();

    //QSGSimpleTextureNode* node = static_cast<QSGSimpleTextureNode*>(oldNode);
    //if (!node) {
    //    node = new QSGSimpleTextureNode();
    //}

    //// �� DirectX ����ת��Ϊ Qt ����
    //if (!m_texture) {
    //    m_texture = window()->createTextureFromNativeObject(
    //        QQuickWindow::NativeObjectTexture,
    //        m_renderTargetTexture,
    //        0,  // Level or plane, 0 for default
    //        QSize(width(), height()),
    //        QQuickWindow::TextureHasAlphaChannel
    //    );
    //}

    //node->setTexture(m_texture);
    //node->setRect(boundingRect());

    return oldNode;
}

void DXItem::render()
{
    if (!m_context || !m_renderTargetView) return;

    FillTextureWithGreen(m_context, m_renderTargetTexture, width(), height());
    // �������ɫ����Ϊ��ɫ
    float clearColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    m_context->ClearRenderTargetView(m_renderTargetView, clearColor);

    // ������ȾĿ��
    m_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);

    // ����ǰ�󻺳���
    m_swapChain->Present(1, 0);
}

void DXItem::FillTextureWithGreen(ID3D11DeviceContext* context, ID3D11Texture2D* texture, int width, int height)
{
    // ����һ��������ɫ��ɫ����������
    std::vector<UINT32> greenData(width * height, 0xFF00FF00); // ARGB ��ʽ����ɫ (0xFF00FF00)

    // �������������Դ
    D3D11_BOX box;
    box.left = 0;
    box.right = width;
    box.top = 0;
    box.bottom = height;
    box.front = 0;
    box.back = 1;

    context->UpdateSubresource(
        texture,
        0,          // ����Դ����
        &box,       // Ŀ������
        greenData.data(), // Դ����
        width * sizeof(UINT32), // �м��
        0           // �м��
    );
}
