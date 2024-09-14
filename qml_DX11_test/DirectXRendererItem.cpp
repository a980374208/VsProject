#include <QQuickItem>
#include "DirectXRendererItem.h" // 假设您已经有了DirectXRenderer的定义

DirectXRendererItem::DirectXRendererItem(QQuickItem* parent)
    : QQuickItem(parent),
    m_directxRenderer(new DXQuickWindow()) // 直接在构造函数中创建DirectXRenderer实例
{
    setFlag(ItemHasContents, false); 
    connect(this, &QQuickItem::widthChanged, this, [&]() {
        QSizeF si = this->size();
        m_directxRenderer->OnResize(si.height(), si.width());
        m_directxRenderer->DrawScene();
        });
    connect(this, &QQuickItem::heightChanged, this, [&]() {
        QSizeF si = this->size();
        m_directxRenderer->OnResize(si.height(), si.width());
        m_directxRenderer->DrawScene();
        });
}

DirectXRendererItem::~DirectXRendererItem()
{
    delete m_directxRenderer; // 清理DirectXRenderer资源
}

void DirectXRendererItem::componentComplete()
{
    QQuickItem::componentComplete();
}

