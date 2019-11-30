#include <nan.h>
#include <node.h>
#include "hackrf.h"
#include "device.h"

using namespace v8;

static void OpenDevice(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  
  int index = (info[0]->Uint32Value(Isolate::GetCurrent()->GetCurrentContext())).FromMaybe(0);

  hackrf_device_list_t* list = hackrf_device_list();
  if (index >= list->devicecount) return Nan::ThrowError("Invalid device index");

  hackrf_device* device;
  hackrf_device_list_open(list, index, &device);
  hackrf_device_list_free(list);

  info.GetReturnValue().Set(Device::NewInstance(device));
}

static void Devices(const v8::FunctionCallbackInfo<v8::Value>& info) {

  Isolate* isolate = Isolate::GetCurrent();
  
  HandleScope scope(isolate);
  hackrf_device_list_t* list = hackrf_device_list();

  Local<Array> devices = Nan::New<Array>(list->devicecount);
  
  for(int i = 0; i < list->devicecount; i++) {
    Local<Object> device = Nan::New<Object>();
    enum hackrf_usb_board_id usb_board_id = list->usb_board_ids[i];
    const char* name = hackrf_usb_board_id_name(usb_board_id);
    device->Set(Nan::New("name").ToLocalChecked(), Nan::New<String>(name).ToLocalChecked());
    device->Set(Nan::New("usbIndex").ToLocalChecked(), Nan::New(list->usb_device_index[i]));
    device->Set(Nan::New("usbBoardId").ToLocalChecked(), Nan::New(usb_board_id));
    if (list->serial_numbers[i])
        device->Set(Nan::New("serialNumber").ToLocalChecked(), Nan::New<String>(list->serial_numbers[i]).ToLocalChecked());    
    else 
        device->Set(Nan::New("serialNumber").ToLocalChecked(), Nan::New<String>("").ToLocalChecked());      
    devices->Set(i, device);
  }

  // Expose the method _open to javascript obect devices to allow e.g. radio_device = devices._open(0)
  Nan::SetMethod(devices, "_open", OpenDevice);
  hackrf_device_list_free(list);
  info.GetReturnValue().Set(devices);

}

// Initialize this addon to be context-aware.
NODE_MODULE_INIT(/*exports, module, context*/) {
  
  Isolate* isolate = context->GetIsolate();

  hackrf_init();
  Device::Init();

  // Expose the method "devices" to JavaScript
  exports->Set(context,
               String::NewFromUtf8(isolate, "devices", NewStringType::kNormal)
                  .ToLocalChecked(),
               FunctionTemplate::New(isolate, Devices)
                  ->GetFunction(context).ToLocalChecked()).FromJust();

}
