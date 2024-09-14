#include <QQuickItem>
#include "DirectXRendererItem.h" // �������Ѿ�����DirectXRenderer�Ķ���

DirectXRendererItem::DirectXRendererItem(QQuickItem* parent)
    : QQuickItem(parent),
    m_directxRenderer(new DXQuickWindow()) // ֱ���ڹ��캯���д���DirectXRendererʵ��
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
    delete m_directxRenderer; // ����DirectXRenderer��Դ
}

void DirectXRendererItem::componentComplete()
{
    QQuickItem::componentComplete();
}

