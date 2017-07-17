/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : usb_desc.c
 *    Description : usb decriptors module
 *
 *    History :
 *    1. Date        : 22, July 2007
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 34964 $
 **************************************************************************/
#include "usb_desc.h"

const UsbStandardDeviceDescriptorStr_t UsbStandardDeviceDescriptorStr =
{
  sizeof(UsbStandardDeviceDescriptorStr_t),   // bLength
  UsbDescriptorDevice,                        // bDescriptorType
  0x200,                                      // bcdUSB
  UsbDeviceClassCommunications,               // bDeviceClass
  0,                                          // bDeviceSubClass
  0,                                          // bDeviceProtocol
  Ep0MaxSize,                                 // bMaxPacketSize0
  0x03EB,                                     // idVendor (ATMEL)
  0x2310,                                     // idProduct (CDC ASF)
  0x0000,                                     // bcdDevice
  iManufacturerStr,                           // iManufacturer
  iProductStr,                                // iProduct
  iSerialNumberStr,                           // iSerialNumber
  1,                                          // bNumConfigurations
};

#pragma data_alignment=4
static const Int8U UsbFS_Cfg0[] =
{
// Configuration Descriptor
  // bLength
  sizeof(UsbStandardConfigurationDescriptor_t),
  // bDescriptorType
  UsbDescriptorConfiguration,
  // wTotalLength
  _2BL(sizeof(UsbStandardConfigurationDescriptor_t)+
  (2 * sizeof(UsbStandardInterfaceDescriptor_t))+
   sizeof(CDC_HeaderFuncDescriptor_t)+
   sizeof(CDC_CallManagementFuncDescriptor_t)+
   sizeof(CDC_AbstrCtrlFuncDescriptor_t)+
  (sizeof(CDC_UnionFuncDescriptor_t)+1)+
  (3 * sizeof(UsbStandardEpDescriptor_t))),
  // bNumInterfaces
  2,
  // bConfigurationValue
  1,
  // iConfiguration
  0,
  // bmAttributes
  UsbConfigurationCommmonAttr,
  // bMaxPower
  UsbConfigPower_mA(200),

// Interface Descriptors AbstractControlModel
  // bLength
  sizeof(UsbStandardInterfaceDescriptor_t),
  // bDescriptorType
  UsbDescriptorInterface,
  // bInterfaceNumber
  CDC_CTRL_INTERFACE_IND,
  // bAlternateSetting
  0,
  // bNumEndpoints
  1,
  // bInterfaceClass
  CommInterfaceClassCode,
  // bInterfaceSubClass
  AbstractControlModel,
  // bInterfaceProtocol
  0,
  // iInterface
  0,

// Header Functional Descriptor
  // bFunctionLength
  sizeof(CDC_HeaderFuncDescriptor_t),
  // bDescriptorType
  CS_INTERFACE,
  // bDescriptorSubtype
  Header,
  // bcdCDC
  _2BL(0x110),

// Union Functional Descriptor
  // bFunctionLength
  sizeof(CDC_UnionFuncDescriptor_t)+1,
  // bDescriptorType
  CS_INTERFACE,
  // bDescriptorSubtype
  Union,
  // bMasterInterface
  CDC_CTRL_INTERFACE_IND,
  // bDataInterface
  CDC_DATA_INTERFACE_IND,

//Call Management Functional Descriptor
  // bFunctionLength
  sizeof(CDC_CallManagementFuncDescriptor_t),
  // bDescriptorType
  CS_INTERFACE,
  // bDescriptorSubtype
  CallManagement,
  // bmCapabilities
  0,
  // bDataInterface
  1,

//ACM Functional Descriptor
  // bFunctionLength
  sizeof(CDC_AbstrCtrlFuncDescriptor_t),
  // bDescriptorType
  CS_INTERFACE,
  // bDescriptorSubtype
  AbstractControlManagement,
  // bmCapabilities
  (2 | (CDC_DEVICE_SUPPORT_BREAK?4:0)),

// Endpoint Descriptors ReportEp
  // bLength
  sizeof(UsbStandardEpDescriptor_t),
  // bDescriptorType
  UsbDescriptorEp,
  // bEndpointAddress
  UsbEpIn(ReportEp>>1),
  // bmAttributes
  UsbEpTransferInterrupt,
  // wMaxPacketSize
  _2BL(ReportEpMaxSize),
  // bInterval
  0x1,

// Interface Descriptors Data Class Interface
  // bLength
  sizeof(UsbStandardInterfaceDescriptor_t),
  // bDescriptorType
  UsbDescriptorInterface,
  // bInterfaceNumber
  CDC_DATA_INTERFACE_IND,
  // bAlternateSetting
  0,
  // bNumEndpoints
  2,
  // bInterfaceClass
  DataInterfaceClassCodes,
  // bInterfaceSubClass
  0,
  // bInterfaceProtocol
  0,
  // iInterface
  0,

// Endpoint Descriptors CommOutEp
  // bLength
  sizeof(UsbStandardEpDescriptor_t),
  // bDescriptorType
  UsbDescriptorEp,
  // bEndpointAddress
  UsbEpOut(CommOutEp>>1),
  // bmAttributes
  UsbEpTransferBulk,
  // wMaxPacketSize
  _2BL(CommOutEpMaxSize),
  // bInterval
  0x00,

// Endpoint Descriptors CommInEp
  // bLength
  sizeof(UsbStandardEpDescriptor_t),
  // bDescriptorType
  UsbDescriptorEp,
  // bEndpointAddress
  UsbEpIn(CommInEp>>1),
  // bmAttributes
  UsbEpTransferBulk,
  // wMaxPacketSize
  _2BL(CommInEpMaxSize),
  // bInterval
  0x00,
  0,
};

static const Int8U * const UsbFS_CfgArray[] =
{
  UsbFS_Cfg0,
  NULL,
};

static const UsbEP_ExtDesc_t UsbEPExt_Cfg0EP1 =
{
  1,                     // Configuration
  UsbEpIn(ReportEp>>1),  // EP address
  {
    EP_SLOT1, FALSE
  }
};

static const UsbEP_ExtDesc_t UsbEPExt_Cfg0EP2 =
{
  1,                     // Configuration
  UsbEpOut(CommOutEp>>1),  // EP address
  {
    EP_SLOT2, FALSE
  }
};

static const UsbEP_ExtDesc_t UsbEPExt_Cfg0EP3 =
{
  1,                     // Configuration
  UsbEpIn(CommInEp>>1),  // EP address
  {
    EP_SLOT3, FALSE
  }
};

static const UsbEP_ExtDesc_t * const UsbFS_EPExtArray[] =
{
  &UsbEPExt_Cfg0EP1,
  &UsbEPExt_Cfg0EP2,
  &UsbEPExt_Cfg0EP3,
  NULL,
};

#pragma data_alignment=4
const Int8U UsbLanguagesStr [] =
{
  // Length of LanguagesStr + 2
  4,
  // Desciptor Type
  UsbDescriptorString,
  // Languages ID
  // Languages1 ID English
  0x09,0x04,
};

#pragma data_alignment=4
const Int8U ManufacturerStrLan1 [] =
{
  24, // length
  UsbDescriptorString,  // Descriptor
  'E',0,'K',0,'i',0,'t',0,'s',0,'Z',0,'o',0,'n',0,'e',0,0,0,0,0, // Str
};

#pragma data_alignment=4
const Int8U ProductStrLan1 [] =
{
  24, //length
  UsbDescriptorString, // Descriptor
  'E',0,'K',0,'Z',0,'-',0,'E',0,'0',0,'8',0,0,0,0,0,0,0,0,0, // Str
};


#pragma data_alignment=4
const Int8U SerialNumberStrLan1 [] =
{
  18, //length
  UsbDescriptorString, // Descriptor
  '0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'2',0, // Str
};

const Int8U * const UsbLanguages1Strings[] =
{
  // iManufacturerStr
  ManufacturerStrLan1,
  // iProductStr
  ProductStrLan1,
  // iSerialNumberStr
  SerialNumberStrLan1,
  // Terminator
  NULL
};

static const Int8U * const * const UsbFS_StringArray[] =
{
  UsbLanguages1Strings,
  NULL,
};

const void * const UsbDescArray[] =
{
  UsbFS_CfgArray,
  UsbFS_EPExtArray,
  UsbLanguagesStr,
  UsbFS_StringArray,
  NULL,
};
