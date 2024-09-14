#ifndef DXQUICKITEM_H
#define DXQUICKITEM_H

#include <QQuickItem>
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <qquickwindow.h>

class DXQuickWindow : public QQuickWindow
{
    Q_OBJECT

public:
    DXQuickWindow(QQuickWindow* parent = nullptr);
    ~DXQuickWindow();
    bool InitDirect3D();        // Direct3D��ʼ��
    void OnResize(int height,int width);
    void DrawScene();

protected:
    HINSTANCE m_hAppInst;        // Ӧ��ʵ�����
    HWND      m_hMainWnd;        // �����ھ��
    bool      m_AppPaused;       // Ӧ���Ƿ���ͣ
    bool      m_Minimized;       // Ӧ���Ƿ���С��
    bool      m_Maximized;       // Ӧ���Ƿ����
    bool      m_Resizing;        // ���ڴ�С�Ƿ�仯
    bool      m_Enable4xMsaa;    // �Ƿ���4�����ز���
    UINT      m_4xMsaaQuality;   // MSAA֧�ֵ������ȼ�



    // ʹ��ģ�����(C++11)��������
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
    // Direct3D 11
    ComPtr<ID3D11Device> m_pd3dDevice;                    // D3D11�豸
    ComPtr<ID3D11DeviceContext> m_pd3dImmediateContext;   // D3D11�豸������
    ComPtr<IDXGISwapChain> m_pSwapChain;                  // D3D11������
    // Direct3D 11.1
    ComPtr<ID3D11Device1> m_pd3dDevice1;                  // D3D11.1�豸
    ComPtr<ID3D11DeviceContext1> m_pd3dImmediateContext1; // D3D11.1�豸������
    ComPtr<IDXGISwapChain1> m_pSwapChain1;                // D3D11.1������
    // ������Դ
    ComPtr<ID3D11Texture2D> m_pDepthStencilBuffer;        // ���ģ�建����
    ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;   // ��ȾĿ����ͼ
    ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;   // ���ģ����ͼ
    D3D11_VIEWPORT m_ScreenViewport;                      // �ӿ�

    // ������Ӧ���ڹ��캯�����ú���Щ�Զ���ĳ�ʼ����
    std::wstring m_MainWndCaption;                       // �����ڱ���
    int m_ClientWidth;                                   // �ӿڿ��
    int m_ClientHeight;                                  // �ӿڸ߶�
private:
};

#endif // DXQUICKITEM_H
