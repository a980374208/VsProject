#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include "dxquickwindow.h"

class DirectXRendererItem : public QQuickItem {
    Q_OBJECT
public:
    DirectXRendererItem(QQuickItem* parent = nullptr);
    ~DirectXRendererItem() override;
protected:
    void componentComplete() override;


private:
    DXQuickWindow* m_directxRenderer; // 改为使用DirectXRenderer
};