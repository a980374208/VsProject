#ifndef DXITEM_H
#define DXITEM_H

#include <QQuickItem>
#include <QSGTexture>
#include <d3d11.h>
#include <dxgi.h>

class QQuickWindow;

class DXItem : public QQuickItem
{
    Q_OBJECT
public:
    DXItem();
    ~DXItem();

protected:
    void releaseResources() override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;

private slots:
    void handleWindowChanged(QQuickWindow* win);
    void handleWindowSizeChanged();

private:
    void initializeDirectX();
    void render();
    void FillTextureWithGreen(ID3D11DeviceContext* context, ID3D11Texture2D* texture, int width, int height);

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    IDXGISwapChain* m_swapChain;
    ID3D11RenderTargetView* m_renderTargetView;
    ID3D11Texture2D* m_renderTargetTexture;
    QSGTexture* m_texture;
};

#endif // DXITEM_H
