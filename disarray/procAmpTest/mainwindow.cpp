#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <dshow.h>
#include <commdlg.h>
#include <wrl/client.h>
#include <ocidl.h>
using Microsoft::WRL::ComPtr;

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("test");
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr)) {
        ICreateDevEnum* pDevEnum = nullptr;
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
                              IID_ICreateDevEnum, reinterpret_cast<void**>(&pDevEnum));
        if (SUCCEEDED(hr)) {
            IEnumMoniker* pEnumCat = nullptr;
            hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
            if (hr == S_OK) {
                IMoniker* pMoniker = nullptr;
                while (pEnumCat->Next(1, &pMoniker, nullptr) == S_OK) {
                    // 根据需要检查设备名称或其他属性以找到特定摄像头
                    // ...

                    // 绑定到所需的摄像头设备
                    IPropertyBag* pPropBag = nullptr;
                    hr = pMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag,
                                                 reinterpret_cast<void**>(&pPropBag));
                    if (SUCCEEDED(hr)) {

                        VARIANT deviceName;
                        deviceName.vt = VT_BSTR;

                        hr = pPropBag->Read(L"FriendlyName", &deviceName, NULL);
                        if (FAILED(hr))
                            return ;


                        VARIANT devicePath;
                        devicePath.vt = VT_BSTR;
                        devicePath.bstrVal = NULL;
                        pPropBag->Read(L"DevicePath", &devicePath, NULL);

                        IBaseFilter* filter;
                        hr = pMoniker->BindToObject(NULL, 0, IID_IBaseFilter,
                                                      (void **)&filter);
                        
                        if (FAILED(hr))
                            return ;

                        VARIANT var;
                        VariantInit(&var);

                        // 获取设备的友好名称
                        hr = pPropBag->Read(L"FriendlyName", &var, 0);
                        ui->comboBox->addItem(QString::fromStdWString(var.bstrVal),QVariant::fromValue((void *)filter));
                        VariantClear(&var);
                        pPropBag->Release();

                    }
                    pMoniker->Release();
                }
                pEnumCat->Release();
            }
            pDevEnum->Release();
        }
        CoUninitialize();
    }

}

MainWindow::~MainWindow()
{
    for (int i = 0; i < ui->comboBox->count(); ++i) {
        IBaseFilter* filter = (IBaseFilter*)ui->comboBox->itemData(i).value<void*>();
        filter->Release();
    }
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    IBaseFilter* filter = (IBaseFilter*)ui->comboBox->currentData().value<void*>();
    ComPtr<ISpecifyPropertyPages> pages;
    filter->QueryInterface(IID_ISpecifyPropertyPages, (void**)&pages);
    CAUUID cauuid;

    if (pages != NULL) {
        if (SUCCEEDED(pages->GetPages(&cauuid)) && cauuid.cElems) {
            OleCreatePropertyFrame(nullptr, 0, 0, NULL, 1,
                (LPUNKNOWN*)&filter,
                cauuid.cElems, cauuid.pElems, 0,
                0, NULL);
            CoTaskMemFree(cauuid.pElems);
        }
    }

    CoUninitialize();
}

