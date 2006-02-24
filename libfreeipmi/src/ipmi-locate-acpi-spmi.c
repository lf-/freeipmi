/* 
   ipmi-locate-acpi-spmi.c - ACPI tables driver to locate IPMI interfaces
   using SPMI table.

   Copyright (C) 2003, 2004, 2005 FreeIPMI Core Team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  
*/

#include "freeipmi-build.h"
#include "err-wrappers.h"
#include "fiid-wrappers.h"

fiid_template_t tmpl_acpi_rsdp_descriptor =  /* Root System Descriptor Pointer */
  {
    /* ACPI signature, contains "RSD PTR " */
    {64, "signature", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},              
    /* To make sum of struct == 0 */
    {8 , "checksum", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},               
    /* OEM identification */
    {48, "oem_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},                 
    /* Must be 0 for 1.0, 2 for 2.0 */
    {8 , "revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},               
    /* 32-bit physical address of RSDT */
    {32, "rsdt_physical_address", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* XSDT Length in bytes including hdr */
    {32, "length", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},                 
    /* 64-bit physical address of XSDT */
    {64, "xsdt_physical_address", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* Checksum of entire table */
    {8 , "extended_checksum", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},      
    /* Reserved field must be 0 */
    {24, "reserved", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},               
    {0, "", 0}
  };

fiid_template_t tmpl_acpi_table_hdr =
  {
    /* ACPI signature (4 ASCII characters) */
    {32, "signature", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},              
    /* Length of table, in bytes, including header */
    {32, "length", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},                 
    /* ACPI Specification minor version # */
    {8,  "revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},               
    /* To make sum of entire table == 0 */
    {8,  "checksum", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},               
    /* OEM identification */
    {48, "oem_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},                 
    /* OEM table identification */
    {64, "oem_table_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},           
    /* OEM revision number */ 
    {32, "oem_revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},           
    /* ASL compiler vendor ID */
    {32, "asl_compiler_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},        
    /* ASL compiler revision number */
    {32, "asl_compiler_revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    {0,  "", 0}
  };

fiid_template_t tmpl_acpi_spmi_table_descriptor = 
  {
    /* `SPMI'. Signature for the Service Processor Management 
       Interface Table. */
    {32, "signature", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Length, in bytes, of the entire Service Processor Management 
       Interface Table. */
    {32, "length", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},    
    /* 5 */
    {8,  "revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* Entire table must sum to zero. */
    {8,  "checksum", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* OEM ID. Per ACPI specification. An OEM-supplied string that 
       identifies the OEM. */
    {48, "oemid", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},    
    /* For the Service Processor Management Interface Table, 
       the table ID is the manufacturer model ID 
       (assigned by the OEM identified by "OEM ID"). */
    {64, "oem_table_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* OEM revision of Service Processor Management Interface
       Table for supplied the given OEM Table ID. Per ACPI, this is
       "An OEM-supplied revision number. Larger numbers are
       assumed to be newer revisions." */
    {32, "oem_revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Vendor ID of utility that created the table. For the tables
       containing Definition Blocks, this is the ID for the ASL
       Compiler. */
    {32, "creator_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},   
    /* Revision of utility that created the table. For the tables
       containing Definition Blocks, this is the revision 
       for the ASL Compiler. */
    {32, "creator_revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Indicates the type of IPMI interface:
       0 Reserved
       1 Keyboard Controller Style (KCS)
       2 Server Management Interface Chip (SMIC)
       3 Block Transfer (BT)
       4 SMBus System Interface (SSIF)
       5-255 Reserved */
    {8, "interface_type", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},    
    /* This field must always be 01h to be compatible with any
       software that implements previous 
       versions of this spec. */
    {8, "ipmi_legacy", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},        
    /* Identifies the IPMI specification revision, 
       in BCD format, to which the interface 
       was designed. The first byte holds the
       most significant digits, while second byte holds 
       the least significant digits of the revision, 
       e.g. a value of 0x0150 indicates the interface is 
       compatible with IPMI version v1.5. */
    /*     {16, "specification_revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  */
    {8, "specification_revision.major", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},
    {8, "specification_revision.minor", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},
    /* Interrupt type(s) used by 
       the interface:
       [0] - SCI triggered through GPE 
       (use 0b for SSIF)
       0 = not supported
       1 = supported */
    {1, "interrupt_type.sci_triggered_thru_gpe", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* [1] - I/O APIC/SAPIC interrupt 
       (Global System Interrupt) */
    {1, "interrupt_type.io_apic_sapic_interrupt", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [7:2] - Reserved (must be 0) */
    {6, "interrupt_type.reserved", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* The bit assignment of the SCI interrupt within the GPEx_STS
       register of a GPE described if the FADT that the interface
       triggers. (Note: This field is valid only if Bit[0] of the Interrupt
       Type field is set. Otherwise set to 00h.) */   
    {8, "gpe", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* 00h */
    {8, "reserved1", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [0] - PCI Device Flag. For PCI IPMI devices, 
       this bit is set. For non-PCI devices, this bit is cleared. 
       When this bit is cleared, the PCI Segment Group, Bus, 
       Device and Function Number fields combined corresponds 
       to the ACPI _UID value of the device whose _HID or _CID 
       contains IPI0001 plug and play ID. 
       _UID must be an integer. Byte 60 contains the least
       significant byte of the _UID value. Set to 0b for SSIF. */
    {1, "pci_device_flag", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [7:1] - Reserved */
    {7, "pci_device_flag.reserved", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* The I/O APIC or I/O SAPIC Global System 
       Interrupt used by the interface. 
       (Note: This field is valid only if Bit[1] of the 
       Interrupt Type field is set. 
       Otherwise set to 00h.) */
    {32, "global_system_interrupt", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* The base address of the interface register 
       set described using the Generic Address Structure 
       (GAS, See [ACPI 2.0] for the
       definition). The Address_Space_ID field in 
       the GAS can only be of the value of 0 (System Memory), 
       1 (System IO), and 4 (SMBus). 
       All other values are not permitted.
       For SSIF:
       The Address_Space_ID = 4 and the address field of the GAS
       holds the 7-bit slave address of the BMC on the host SMBus
       in the least significant byte. Note that the slave address is
       stored with the 7-bit slave address in the least 
       significant 7-
       bits of the byte, and the most significant 
       bit of the byte set to
       0b.
       Register_Bit_Width = 0
       Register_Bit_Offset = 0
       Address_Size field = 1 (Byte access)
       Address = 7-bit SMBus address of BMC
       SSIF */
    /*     {96, "base_address", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  */
    /* Address space where
       struct or register
       exists. */  
    {8,  "base_address.address_space_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},    
    /* Size in bits of given
       register */ 
    {8,  "base_address.register_bit_width", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Bit offset within the
       register */ 
    {8,  "base_address.register_bit_offset", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Must be 0 */
    {8,  "base_address.reserved", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},            
    /* 64-bit address of
       struct or register */ 
    {64, "base_address.address", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},       
    /* for IPMI cards not located on PCI */
    {32, "uid", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* This field must always be null (0x00) to be compatible with
       any software that implements previous versions of this spec.
       This field is a deprecated "SPMI ID Field". Implementations
       based on pre-IPMI v2.0 versions of SPMI may contain a null-
       terminated string here. */
    {8, "reserved2", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    {0,  "", 0}
  };

fiid_template_t tmpl_acpi_spmi_table_descriptor_ssif = 
  {
    /* `SPMI'. Signature for the Service Processor Management 
       Interface Table. */
    {32, "signature", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Length, in bytes, of the entire Service Processor Management 
       Interface Table. */
    {32, "length", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},    
    /* 5 */
    {8,  "revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* Entire table must sum to zero. */
    {8,  "checksum", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* OEM ID. Per ACPI specification. An OEM-supplied string that 
       identifies the OEM. */
    {48, "oemid", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},     
    /* For the Service Processor Management Interface Table, 
       the table ID is the manufacturer model ID 
       (assigned by the OEM identified by "OEM ID"). */
    {64, "oem_table_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},
    /* OEM revision of Service Processor Management Interface
       Table for supplied the given OEM Table ID. Per ACPI, this is
       "An OEM-supplied revision number. Larger numbers are
       assumed to be newer revisions." */
    {32, "oem_revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Vendor ID of utility that created the table. For the tables
       containing Definition Blocks, this is the ID for the ASL
       Compiler. */
    {32, "creator_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* Revision of utility that created the table. For the tables
       containing Definition Blocks, this is the revision 
       for the ASL Compiler. */
    {32, "creator_revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Indicates the type of IPMI interface:
       0 Reserved
       1 Keyboard Controller Style (KCS)
       2 Server Management Interface Chip (SMIC)
       3 Block Transfer (BT)
       4 SMBus System Interface (SSIF)
       5-255 Reserved */
    {8, "interface_type", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},    
    /* This field must always be 01h to be compatible with any
       software that implements previous 
       versions of this spec. */
    {8, "ipmi_legacy", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},       
    /* Identifies the IPMI specification revision, 
       in BCD format, to which the interface 
       was designed. The first byte holds the
       most significant digits, while second byte holds 
       the least significant digits of the revision, 
       e.g. a value of 0x0150 indicates the interface is 
       compatible with IPMI version v1.5. */
    /* {16, "specification_revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  */
    {8, "specification_revision.major", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},
    {8, "specification_revision.minor", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},
    /* Interrupt type(s) used by 
       the interface:
       [0] - SCI triggered through GPE 
       (use 0b for SSIF)
       0 = not supported
       1 = supported */
    {1, "interrupt_type.sci_triggered_thru_gpe", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* [1] - I/O APIC/SAPIC interrupt 
       (Global System Interrupt) */
    {1, "interrupt_type.io_apic_sapic_interrupt", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [7:2] - Reserved (must be 0) */
    {6, "interrupt_type.reserved", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* The bit assignment of the SCI interrupt within the GPEx_STS
       register of a GPE described if the FADT that the interface
       triggers. (Note: This field is valid only if Bit[0] of the Interrupt
       Type field is set. Otherwise set to 00h.) */
    {8, "gpe", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* 00h */
    {8, "reserved1", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [0] - PCI Device Flag. For PCI IPMI devices, 
       this bit is set. For non-PCI devices, this bit is cleared. 
       When this bit is cleared, the PCI Segment Group, Bus, 
       Device and Function Number fields combined corresponds 
       to the ACPI _UID value of the device whose _HID or _CID 
       contains IPI0001 plug and play ID. 
       _UID must be an integer. Byte 60 contains the least
       significant byte of the _UID value. Set to 0b for SSIF. */
    {1, "pci_device_flag", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [7:1] - Reserved */
    {7, "pci_device_flag.reserved", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* The I/O APIC or I/O SAPIC Global System 
       Interrupt used by the interface. 
       (Note: This field is valid only if Bit[1] of the 
       Interrupt Type field is set. 
       Otherwise set to 00h.) */
    {32, "global_system_interrupt", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* The base address of the interface register 
       set described using the Generic Address Structure 
       (GAS, See [ACPI 2.0] for the
       definition). The Address_Space_ID field in 
       the GAS can only be of the value of 0 (System Memory), 
       1 (System IO), and 4 (SMBus). 
       All other values are not permitted.
       For SSIF:
       The Address_Space_ID = 4 and the address field of the GAS
       holds the 7-bit slave address of the BMC on the host SMBus
       in the least significant byte. Note that the slave address is
       stored with the 7-bit slave address in the least 
       significant 7-
       bits of the byte, and the most significant 
       bit of the byte set to
       0b.
       Register_Bit_Width = 0
       Register_Bit_Offset = 0
       Address_Size field = 1 (Byte access)
       Address = 7-bit SMBus address of BMC
       SSIF */
    /*     {96, "base_address", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  */
    /* Address space where
       struct or register
       exists. */  
    {8,  "base_address.address_space_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},    
    /* Size in bits of given
       register */ 
    {8,  "base_address.register_bit_width", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* Bit offset within the
       register */ 
    {8,  "base_address.register_bit_offset", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* field = 1 (byte
       access) */
    {8,  "base_address.address_size", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},        
    /* SMBus address of BMC
       SSIF */
    {7, "base_address.address", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},              
    /* Reserved */
    {57, "base_address.reserved", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},            
    /* for IPMI cards not located on PCI */
    {32, "uid", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* This field must always be null (0x00) to be compatible with
       any software that implements previous versions of this spec.
       This field is a deprecated "SPMI ID Field". Implementations
       based on pre-IPMI v2.0 versions of SPMI may contain a null-
       terminated string here. */
    {8, "reserved2", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    {0,  "", 0}
  };

fiid_template_t tmpl_acpi_spmi_table_descriptor_pci_ipmi = 
  {
    /* `SPMI'. Signature for the Service Processor Management 
       Interface Table. */
    {32, "signature", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Length, in bytes, of the entire Service Processor Management 
       Interface Table. */
    {32, "length", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},    
    /* 5 */
    {8,  "revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* Entire table must sum to zero. */
    {8,  "checksum", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* OEM ID. Per ACPI specification. An OEM-supplied string that 
       identifies the OEM. */
    {48, "oemid", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},     
    /* For the Service Processor Management Interface Table, 
       the table ID is the manufacturer model ID 
       (assigned by the OEM identified by "OEM ID"). */
    {64, "oem_table_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* OEM revision of Service Processor Management Interface
       Table for supplied the given OEM Table ID. Per ACPI, this is
       "An OEM-supplied revision number. Larger numbers are
       assumed to be newer revisions." */
    {32, "oem_revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Vendor ID of utility that created the table. For the tables
       containing Definition Blocks, this is the ID for the ASL
       Compiler. */
    {32, "creator_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},   
    /* Revision of utility that created the table. For the tables
       containing Definition Blocks, this is the revision 
       for the ASL Compiler. */
    {32, "creator_revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Indicates the type of IPMI interface:
       0 Reserved
       1 Keyboard Controller Style (KCS)
       2 Server Management Interface Chip (SMIC)
       3 Block Transfer (BT)
       4 SMBus System Interface (SSIF)
       5-255 Reserved */
    {8, "interface_type", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},    
    /* This field must always be 01h to be compatible with any
       software that implements previous 
       versions of this spec. */
    {8, "ipmi_legacy", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},       
    /* Identifies the IPMI specification revision, 
       in BCD format, to which the interface 
       was designed. The first byte holds the
       most significant digits, while second byte holds 
       the least significant digits of the revision, 
       e.g. a value of 0x0150 indicates the interface is 
       compatible with IPMI version v1.5. */
    /* {16, "specification_revision", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, */ 
    {8, "specification_revision.major", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},
    {8, "specification_revision.minor", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},
    /* Interrupt type(s) used by 
       the interface:
       [0] - SCI triggered through GPE 
       (use 0b for SSIF)
       0 = not supported
       1 = supported */
    {1, "interrupt_type.sci_triggered_thru_gpe", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* [1] - I/O APIC/SAPIC interrupt 
       (Global System Interrupt) */
    {1, "interrupt_type.io_apic_sapic_interrupt", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [7:2] - Reserved (must be 0) */
    {6, "interrupt_type.reserved", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* The bit assignment of the SCI interrupt within the GPEx_STS
       register of a GPE described if the FADT that the interface
       triggers. (Note: This field is valid only if Bit[0] of the Interrupt
       Type field is set. Otherwise set to 00h.) */
    {8, "gpe", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* 00h */
    {8, "reserved1", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [0] - PCI Device Flag. For PCI IPMI devices, 
       this bit is set. For non-PCI devices, this bit is cleared. 
       When this bit is cleared, the PCI Segment Group, Bus, 
       Device and Function Number fields combined corresponds 
       to the ACPI _UID value of the device whose _HID or _CID 
       contains IPI0001 plug and play ID. 
       _UID must be an integer. Byte 60 contains the least
       significant byte of the _UID value. Set to 0b for SSIF. */
    {1, "pci_device_flag", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [7:1] - Reserved */
    {7,  "pci_device_flag.reserved", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* The I/O APIC or I/O SAPIC Global System 
       Interrupt used by the interface. 
       (Note: This field is valid only if Bit[1] of the 
       Interrupt Type field is set. 
       Otherwise set to 00h.) */
    {32, "global_system_interrupt", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},  
    /* The base address of the interface register 
       set described using the Generic Address Structure 
       (GAS, See [ACPI 2.0] for the
       definition). The Address_Space_ID field in 
       the GAS can only be of the value of 0 (System Memory), 
       1 (System IO), and 4 (SMBus). 
       All other values are not permitted.
       For SSIF:
       The Address_Space_ID = 4 and the address field of the GAS
       holds the 7-bit slave address of the BMC on the host SMBus
       in the least significant byte. Note that the slave address is
       stored with the 7-bit slave address in the least 
       significant 7-
       bits of the byte, and the most significant 
       bit of the byte set to
       0b.
       Register_Bit_Width = 0
       Register_Bit_Offset = 0
       Address_Size field = 1 (Byte access)
       Address = 7-bit SMBus address of BMC SSIF */
    /* {96, "base_address"} */ 
    /* Address space where
       struct or register
       exists. */  
    {8,  "base_address.address_space_id", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},    
    /* Size in bits of given
       register */ 
    {8,  "base_address.register_bit_width", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Bit offset within the
       register */ 
    {8,  "base_address.register_bit_offset", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* Must be 0 */
    {8,  "base_address.reserved", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},            
    /* 64-bit address of
       struct or register */ 
    {64, "base_address.address", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED},
    /* PCI Segment Group Number, 
       if the IPMI device is a PCI
       device. Otherwise, this field is byte 1 of a UID. 
       See description for PCI Device Flag, above. */
    {8, "pci_segment_group_number", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* PCI Bus Number, if the IPMI device is a PCI device.
       Otherwise, this field is byte 2 of a UID. 
       See description for PCI Device Flag, above. */
    {8, "pci_bus_number", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* PCI Device fields or byte 3 of a UID. Per PCI Device Flag,
       above.
       For PCI Device Flag = 1b:
       [4:0] - PCI Device Number: The PCI device number if the
       IPMI device is a PCI device.
       For PCI Device Flag = 0b:
       [7:0] - byte 3 of UID */
    {4, "pci_device_number", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [7:5] - Reserved */
    {4, "pci_device_number.reserved", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* PCI Device fields or byte 4 of a UID. 
       Per PCI Device Flag, above.
       For PCI Device Flag = 1b:
       [2:0] - PCI Function Number: The PCI function number if
       the IPMI device is a PCI device. */
    {3, "pci_function_number", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [5:3] - Reserved */
    {3, "pci_function_number.reserved1", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [6] -    Interrupt Flag:
       0b = interrupt not supported
       1b = interrupt supported */
    {1, "pci_function_number.interrupt_flag", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* [7] -    Reserved 
       For PCI Device Flag = 0b:
       [7:0] - byte 4 of UID */
    {1, "pci_function_number.reserved2", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    /* This field must always be null (0x00) to be compatible with
       any software that implements previous versions of this spec.
       This field is a deprecated "SPMI ID Field". Implementations
       based on pre-IPMI v2.0 versions of SPMI may contain a null-
       terminated string here. */
    {8, "reserved2", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
    {0,  "", 0}
  };

static uint8_t ipmi_acpi_table_checksum (uint8_t *buffer, size_t len);
static int ipmi_acpi_get_rsdp (uint64_t rsdp_window_base_addr, 
			       size_t rsdp_window_size,
			       fiid_obj_t obj_acpi_rsdp_descriptor);
static int ipmi_acpi_get_table (uint64_t table_address, 
				char *signature,
				uint8_t **acpi_table, 
				uint32_t *acpi_table_length);
static int ipmi_acpi_get_firmware_table (char *signature, 
					 int table_instance,
					 fiid_obj_t obj_acpi_table_hdr,
					 uint8_t **sign_table_data,
					 uint32_t *sign_table_data_length);
static int ipmi_acpi_get_spmi_table (uint8_t interface_type,
				     fiid_obj_t obj_acpi_table_hdr,
				     fiid_obj_t obj_acpi_spmi_table_descriptor);

/*******************************************************************************
 *
 * FUNCTION:
 *   ipmi_acpi_table_checksum
 *
 * PARAMETERS:
 *   buffer  - Buffer to checksum
 *   length  - Size of the buffer
 *
 * RETURNS:
 *   8 bit checksum of buffer. NON-ZERO = checksum failed.
 *
 * DESCRIPTION:
 *   Computes an 8 bit checksum of the buffer(length) and returns it.
 *
 ******************************************************************************/
static uint8_t 
ipmi_acpi_table_checksum (uint8_t *buffer, size_t len)
{
  int i = 0;
  uint8_t sum = 0;
  
  if (buffer == NULL)
    return 0;
  
  for (i = 0; i < len; i++)
    sum += buffer[i];
  
  return sum;
}

static int
ipmi_ioremap (uint64_t physical_addr, size_t physical_addr_len,
              void **virtual_addr,
              void **mapped_addr, size_t *mapped_addr_len)
{
  uint64_t startaddr;
  uint32_t pad;
  int mem_fd;
  extern int errno;

  if (!(physical_addr_len && virtual_addr &&
        mapped_addr && mapped_addr_len))
    {
      errno = EINVAL;
      return (-1);
    }

  if ((mem_fd = open ("/dev/mem", O_RDONLY|O_SYNC)) == -1)
    return (-1);

  pad = physical_addr % getpagesize ();
  startaddr = physical_addr - pad;
  *mapped_addr_len = physical_addr_len + pad;
  *mapped_addr = mmap (NULL, *mapped_addr_len, PROT_READ, MAP_PRIVATE, mem_fd, startaddr);


  if (*mapped_addr == MAP_FAILED)
    {
      close (mem_fd);
      return (-1);
    }

  close (mem_fd);
  *virtual_addr = (*mapped_addr) + pad;
  return (0);
}

static int
ipmi_iounmap (void *mapped_addr, size_t mapped_addr_len)
{
  return (munmap (mapped_addr, mapped_addr_len));
}

static int
ipmi_get_physical_mem_data (uint64_t physical_address,
                            size_t length,
                            uint8_t *data)
{
  void *virtual_addr = NULL;
  void *mapped_addr = NULL;
  size_t mapped_addr_len = 0;

  if (data == NULL)
    {
      errno = EINVAL;
      return (-1);
    }

  if (ipmi_ioremap (physical_address, length,
                    &virtual_addr,
                    &mapped_addr, &mapped_addr_len) != 0)
    return (-1);

  memcpy (data, virtual_addr, length);

  ipmi_iounmap (mapped_addr, mapped_addr_len);

  return 0;
}

/*******************************************************************************
 *
 * FUNCTION:
 *   ipmi_acpi_get_rsdp
 *
 * PARAMETERS:
 *   rsdp_window_base_addr  - Starting pointer for search
 *   rsdp_window_wize       - Maximum length to search
 *   obj_acpi_rsdp_descriptor  - Initialized rsdp descriptor object
 *
 * RETURN:
 *   A return value of 0 means success. RSDP descriptor is returned
 *   through obj_acpi_rsdp_descriptor parameter.
 *
 * DESCRIPTION:
 *   Search a block of memory for the RSDP signature.
 *
 * NOTE:
 *   The RSDP must be either in the first 1_k of the Extended BIOS
 *   Data Area or between E0000 and FFFFF (ACPI 1.0 section 5.2.2;
 *   assertion #421).
 *
 ******************************************************************************/
static int 
ipmi_acpi_get_rsdp (uint64_t rsdp_window_base_addr, size_t rsdp_window_size, 
		    fiid_obj_t obj_acpi_rsdp_descriptor)
{
  uint8_t *memdata = NULL;
  int acpi_rsdp_descriptor_len;
  int i;

  if (!fiid_obj_valid(obj_acpi_rsdp_descriptor))
    {
      errno = EINVAL;
      return (-1);
    }

  FIID_OBJ_TEMPLATE_COMPARE(obj_acpi_rsdp_descriptor, tmpl_acpi_rsdp_descriptor);
  
  memdata = alloca (rsdp_window_size);
  memset (memdata, 0, rsdp_window_size);
  
  FIID_TEMPLATE_LEN_BYTES (acpi_rsdp_descriptor_len,
			   tmpl_acpi_rsdp_descriptor);
  
  if (ipmi_get_physical_mem_data (rsdp_window_base_addr, 
				  rsdp_window_size, memdata) != 0)
    return (-1);
  
  /* Search from given start addr for the requested length  */
  for (i = 0; i < rsdp_window_size; i += IPMI_ACPI_RSDP_SCAN_STEP)
    {
      /* check RSDP signature */
      if (strncmp ((char *)&memdata[i], 
		   IPMI_ACPI_RSDP_SIG, 
		   strlen (IPMI_ACPI_RSDP_SIG)) != 0)
	continue;
      
      /* now check the checksum */
      if (ipmi_acpi_table_checksum (&memdata[i], 
				    IPMI_ACPI_RSDP_CHECKSUM_LENGTH) == 0)
	{
	  FIID_OBJ_SET_ALL(obj_acpi_rsdp_descriptor,
			   &memdata[i], 
			   acpi_rsdp_descriptor_len);
	  
	  /* check this RSDP has RSDT/XSDT */
	  {
	    uint64_t val;
	    uint8_t revision;
	    uint64_t rsdt_xsdt_address;
	    char *rsdt_xsdt_signature;
	    uint8_t *rsdt_xsdt_table = NULL;
	    uint32_t rsdt_xsdt_table_length;
	    
	    FIID_OBJ_GET (obj_acpi_rsdp_descriptor, 
			  (uint8_t *)"revision", 
			  &val);

	    revision = val;
	    if (revision < 2)
	      { 
		FIID_OBJ_GET (obj_acpi_rsdp_descriptor, 
			      (uint8_t *)"rsdt_physical_address", 
			      &rsdt_xsdt_address);

		rsdt_xsdt_signature = IPMI_ACPI_RSDT_SIG;
	      }
	    else 
	      {
		FIID_OBJ_GET (obj_acpi_rsdp_descriptor, 
			      (uint8_t *)"xsdt_physical_address", 
			      &rsdt_xsdt_address);

		rsdt_xsdt_signature = IPMI_ACPI_XSDT_SIG;
	      }
	    
	    if (ipmi_acpi_get_table (rsdt_xsdt_address, 
				     rsdt_xsdt_signature, 
				     &rsdt_xsdt_table, 
				     &rsdt_xsdt_table_length) == 0)
	      {
		/* we found RSDT/XSDT */
		free (rsdt_xsdt_table);
		return 0;
	      }
	    free (rsdt_xsdt_table);
	    
	    /* This is special case because of EFI */
	    FIID_OBJ_GET (obj_acpi_rsdp_descriptor, 
			  (uint8_t *)"rsdt_physical_address", 
			  &rsdt_xsdt_address);

	    memdata = alloca (acpi_rsdp_descriptor_len);
	    memset (memdata, 0, acpi_rsdp_descriptor_len);
	    if (ipmi_get_physical_mem_data (rsdt_xsdt_address, 
					    acpi_rsdp_descriptor_len, 
					    memdata) != 0)
	      return (-1);
	    
	    /* check RSDP signature */
	    if (strncmp ((char *)memdata, 
			 IPMI_ACPI_RSDP_SIG, 
			 strlen (IPMI_ACPI_RSDP_SIG)) != 0)
	      return (-1);
	    
	    /* now check the checksum */
	    if (ipmi_acpi_table_checksum (memdata, 
					  IPMI_ACPI_RSDP_CHECKSUM_LENGTH) != 0)
	      return (-1);
	    
	    /* we found another RSDP */
	    memcpy (obj_acpi_rsdp_descriptor, memdata, acpi_rsdp_descriptor_len);
	  }
	  
	  return 0;
	}
    }
  
  return (-1);
}

/*******************************************************************************
 *
 * FUNCTION:
 *   ipmi_acpi_get_table
 *
 * PARAMETERS:
 *   table_address     - ACPI table physical address
 *   signature         - signature of the table
 *   acpi_table        - ACPI table in malloc'ed memory
 *   acpi_table_length - ACPI table length
 *
 * RETURN:
 *   A return value of 0 means success. ACPI table (including header) is
 *   returned through acpi_table parameter.
 *
 * DESCRIPTION:
 *   Retrieve any ACPI table (including header) pointed by table address.
 *
 ******************************************************************************/
static int 
ipmi_acpi_get_table (uint64_t table_address, char *signature, 
		     uint8_t **acpi_table, uint32_t *acpi_table_length)
{
  uint64_t val;
  
  uint8_t table_signature_length;
  char *table_signature;
  
  fiid_obj_t obj_acpi_table_hdr = NULL;
  uint8_t *acpi_table_buf;
  uint32_t acpi_table_hdr_length;
  uint32_t table_length = 0;
  uint8_t *table;
  int32_t len;
  int rv = -1;

  if (signature == NULL || acpi_table == NULL || acpi_table_length == NULL)
    return (-1);

  FIID_TEMPLATE_FIELD_LEN_BYTES_CLEANUP(len, 
					tmpl_acpi_table_hdr, 
					(uint8_t *)"signature");

  table_signature_length = len + 1;
  table_signature = alloca (table_signature_length);
  memset (table_signature, 0, table_signature_length);
  
  FIID_TEMPLATE_LEN_BYTES_CLEANUP (acpi_table_hdr_length,
				   tmpl_acpi_table_hdr);

  FIID_OBJ_CREATE_CLEANUP(obj_acpi_table_hdr, tmpl_acpi_table_hdr);

  acpi_table_buf = alloca (acpi_table_hdr_length);

  memset (acpi_table_buf, 0, acpi_table_hdr_length);
  
  if (ipmi_get_physical_mem_data (table_address, 
				  acpi_table_hdr_length, 
				  acpi_table_buf) != 0)
    goto cleanup;
  
  FIID_OBJ_SET_ALL_CLEANUP(obj_acpi_table_hdr,
			   acpi_table_buf,
			   acpi_table_hdr_length);

  if (fiid_obj_get_data (obj_acpi_table_hdr, 
			 (uint8_t *)"signature", 
			 (uint8_t *)table_signature, 
			 table_signature_length) < 0)
    goto cleanup;

  if (strcmp (table_signature, signature) != 0)
    goto cleanup;
  
  FIID_OBJ_GET_CLEANUP (obj_acpi_table_hdr, (uint8_t *)"length", &val);
  table_length = val;
  
  table = alloca (table_length);
  memset (table, 0, table_length);
  if (ipmi_get_physical_mem_data (table_address, 
				  table_length, 
				  table) != 0)
    goto cleanup;
  
  if (ipmi_acpi_table_checksum (table, table_length) != 0)
    goto cleanup;
  
  *acpi_table = malloc (table_length);
  memcpy (*acpi_table, table, table_length);
  *acpi_table_length = table_length;
  
  rv = 0;
 cleanup:
  FIID_OBJ_DESTROY_NO_RETURN(obj_acpi_table_hdr);
  return (rv);
}

/*******************************************************************************
 *
 * FUNCTION:
 *   ipmi_acpi_get_firmware_table
 *
 * PARAMETERS:
 *   signature               - ACPI signature for firmware table header
 *   table_instance          - Which instance of the firmware table
 *   obj_acpi_table_hdr      - Initialized ACPI table header
 *   sign_table_data         - Initialized with malloc'ed ACPI firmware table data
 *   sign_table_data_length  - ACPI table DATA length
 *
 * RETURN:
 *   Return 0 for success. ACPI table header and firmware table DATA are
 *   returned through obj_acpi_table_hdr and signed_table_data
 *   parameters.
 *
 * DESCRIPTION:
 *   Top level call for any ACPI firmware table by table signature string.
 *   It gets table header and data from RSDT/XSDT.
 *
 ******************************************************************************/
static int 
ipmi_acpi_get_firmware_table (char *signature, int table_instance, 
			      fiid_obj_t obj_acpi_table_hdr, 
			      uint8_t **sign_table_data, 
			      uint32_t *sign_table_data_length)
{
  uint64_t val;
  
  int32_t acpi_table_hdr_length;
  int32_t acpi_rsdp_descriptor_length;

  fiid_obj_t obj_acpi_rsdp_descriptor = NULL;
  uint64_t rsdt_xsdt_address;
  char *rsdt_xsdt_signature;
  uint8_t revision;
  
  uint8_t *rsdt_xsdt_table = NULL;
  uint32_t rsdt_xsdt_table_length;
  uint8_t *rsdt_xsdt_table_data;
  uint32_t rsdt_xsdt_table_data_length;
  int acpi_table_count;
  uint8_t *acpi_table = NULL;
  uint32_t acpi_table_length;
  
  fiid_obj_t obj_table = NULL;
  uint64_t table_address;
  int signature_table_count;
  int i;
  int rv = -1;
  fiid_template_t tmpl_table_address_32 =
    {
      {32, "table_address", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
      {0,  "", 0}
    };
  fiid_template_t tmpl_table_address_64 =
    {
      {64, "table_address", FIID_FIELD_REQUIRED | FIID_FIELD_LENGTH_FIXED}, 
      {0,  "", 0}
    };

  if (signature == NULL 
      || !fiid_obj_valid(obj_acpi_table_hdr)
      || sign_table_data == NULL 
      || sign_table_data_length == NULL)
    return (-1);
  
  FIID_OBJ_TEMPLATE_COMPARE(obj_acpi_table_hdr, tmpl_acpi_table_hdr);

  FIID_TEMPLATE_LEN_BYTES_CLEANUP (acpi_table_hdr_length, tmpl_acpi_table_hdr);

  FIID_OBJ_CREATE_CLEANUP(obj_acpi_rsdp_descriptor, tmpl_acpi_rsdp_descriptor);

  FIID_TEMPLATE_LEN_BYTES_CLEANUP (acpi_rsdp_descriptor_length, 
				   tmpl_acpi_rsdp_descriptor);

  if (ipmi_acpi_get_rsdp (IPMI_ACPI_LO_RSDP_WINDOW_BASE,
			  IPMI_ACPI_LO_RSDP_WINDOW_SIZE,
			  obj_acpi_rsdp_descriptor) != 0)
    {
      ERR_CLEANUP (!(ipmi_acpi_get_rsdp (IPMI_ACPI_HI_RSDP_WINDOW_BASE,
					 IPMI_ACPI_HI_RSDP_WINDOW_SIZE,
					 obj_acpi_rsdp_descriptor) != 0));
    }
  
  FIID_OBJ_GET_CLEANUP (obj_acpi_rsdp_descriptor, 
			(uint8_t *)"revision", 
			&val);

  revision = val;
  if (revision < 2)
    { 
      FIID_OBJ_GET_CLEANUP (obj_acpi_rsdp_descriptor, 
			    (uint8_t *)"rsdt_physical_address", 
			    &rsdt_xsdt_address);
      rsdt_xsdt_signature = IPMI_ACPI_RSDT_SIG;
    }
  else 
    {
      FIID_OBJ_GET_CLEANUP (obj_acpi_rsdp_descriptor, 
			    (uint8_t *)"xsdt_physical_address", 
			    &rsdt_xsdt_address);
      rsdt_xsdt_signature = IPMI_ACPI_XSDT_SIG;
    }
  
  ERR_CLEANUP (!(ipmi_acpi_get_table (rsdt_xsdt_address, rsdt_xsdt_signature, 
				      &rsdt_xsdt_table, 
				      &rsdt_xsdt_table_length) != 0));
  
  rsdt_xsdt_table_data_length = rsdt_xsdt_table_length - acpi_table_hdr_length;
  rsdt_xsdt_table_data = (rsdt_xsdt_table + acpi_table_hdr_length);
  
  if (revision < 2)
    acpi_table_count = rsdt_xsdt_table_data_length / 4;
  else 
    acpi_table_count = rsdt_xsdt_table_data_length / 8;
  
  acpi_table = NULL;
  acpi_table_length = 0;
  for (i = 0, signature_table_count = 0; i < acpi_table_count; i++)
    {
      fiid_field_t *tmpl_table_address = NULL;
      int32_t len_table;
      
      if (revision < 2)
	tmpl_table_address = &tmpl_table_address_32[0];
      else
	tmpl_table_address = &tmpl_table_address_64[0];
      
      FIID_OBJ_CREATE_CLEANUP(obj_table, tmpl_table_address);
      
      FIID_TEMPLATE_LEN_BYTES_CLEANUP (len_table, tmpl_table_address);
	  
      FIID_OBJ_SET_ALL_CLEANUP(obj_table,
			       (rsdt_xsdt_table_data + (i * 4)),
			       len_table);

      FIID_OBJ_GET_CLEANUP (obj_table,
			    (uint8_t *)"table_address", 
			    &table_address);
      
      FIID_OBJ_DESTROY_NO_RETURN(obj_table);

      if (ipmi_acpi_get_table (table_address, 
			       signature, 
			       &acpi_table, 
			       &acpi_table_length) != 0)
	continue;
      
      signature_table_count++;
      if (signature_table_count == table_instance)
	break;
      
      free (acpi_table);
      acpi_table = NULL;
      acpi_table_length = 0;
    }
  
  free (rsdt_xsdt_table);
  
  ERR_CLEANUP (acpi_table != NULL);
  
  memcpy (obj_acpi_table_hdr, acpi_table, acpi_table_hdr_length);
  *sign_table_data_length = acpi_table_length - acpi_table_hdr_length;
  *sign_table_data = malloc (*sign_table_data_length);
  memcpy (*sign_table_data, (acpi_table + acpi_table_hdr_length), *sign_table_data_length);
  
  rv = 0;
 cleanup:
  if (acpi_table)
    free(acpi_table);
  if (rsdt_xsdt_table)
    free(rsdt_xsdt_table);
  FIID_OBJ_DESTROY_NO_RETURN(obj_table);
  FIID_OBJ_DESTROY_NO_RETURN(obj_acpi_rsdp_descriptor);
  return (rv);
}

/*******************************************************************************
 *
 * FUNCTION:
 *   ipmi_acpi_get_spmi_table
 *
 * PARAMETERS:
 *   interface_type           - Type of interface to look for (KCS, SSIF, SMIC, BT)
 *   obj_acpi_table_hdr       - Initialized ACPI table header
 *   acpi_table_firmware      - Initialized ACPI firmware table
 *
 * RETURN:
 *   Return 0 for success. ACPI table header and SPMI table is
 *   returned through obj_acpi_table_hdr and obj_acpi_spmi_table_descriptor
 *   parameters.
 *
 * DESCRIPTION:
 *   Get SPMI table for the given interface type.
 *
 ******************************************************************************/
static int 
ipmi_acpi_get_spmi_table (uint8_t interface_type,
			  fiid_obj_t obj_acpi_table_hdr,
			  fiid_obj_t obj_acpi_spmi_table_descriptor)
{
  uint64_t val;
  uint8_t table_interface_type; 
  uint8_t *table_data = NULL;
  uint32_t table_data_length = 0;
  uint32_t copy_length;
  int instance;
  fiid_obj_t obj_acpi_table_hdr2 = NULL;
  int32_t acpi_spmi_table_descriptor_len;
  int rv = -1;

  if (!fiid_obj_valid(obj_acpi_table_hdr)
      || !fiid_obj_valid(obj_acpi_spmi_table_descriptor))
    {
      errno = EINVAL;
      return (-1);
    }

  FIID_OBJ_TEMPLATE_COMPARE(obj_acpi_table_hdr, tmpl_acpi_table_hdr);
  FIID_OBJ_TEMPLATE_COMPARE(obj_acpi_spmi_table_descriptor, tmpl_acpi_spmi_table_descriptor);

  for (instance = 0; instance <= IPMI_INTERFACE_MAX; instance++)
    {
      if (ipmi_acpi_get_firmware_table (IPMI_ACPI_SPMI_SIG, instance, 
					obj_acpi_table_hdr2,
					&table_data, 
					&table_data_length) != 0)
	continue;
      
      printf ("__DEBUG__ instance = %d, signature = [%s] found\n", 
	      instance, IPMI_ACPI_SPMI_SIG);

      FIID_TEMPLATE_LEN_BYTES_CLEANUP (acpi_spmi_table_descriptor_len, tmpl_acpi_spmi_table_descriptor);

      if (acpi_spmi_table_descriptor_len < table_data_length)
	copy_length = acpi_spmi_table_descriptor_len;
      else 
	copy_length = table_data_length;
      
      if (copy_length != table_data_length)
	printf ("_DEBUG_ table_data_length=%d, template_length=%d,"
		" tmpl_acpi_spmi_table_descriptor length is too short\n", 
		table_data_length, acpi_spmi_table_descriptor_len);
      
      FIID_OBJ_SET_ALL_CLEANUP(obj_acpi_spmi_table_descriptor,
			       table_data,
			       copy_length);
      
      free (table_data);
      table_data = NULL;
      table_data_length = 0;
      
      FIID_OBJ_GET_CLEANUP (obj_acpi_spmi_table_descriptor, 
			    (uint8_t *)"interface_type", 
			    &val);

      table_interface_type = val;
      if (table_interface_type == interface_type)
	rv = 0;
    }

 cleanup:
  if (table_data)
    free(table_data);
  FIID_OBJ_DESTROY_NO_RETURN(obj_acpi_table_hdr2);
  return (rv);
}

ipmi_locate_info_t*
ipmi_locate_acpi_spmi_get_dev_info (ipmi_interface_type_t type)
{
  fiid_obj_t obj_acpi_table_hdr = NULL;
  fiid_obj_t obj_acpi_spmi_table_descriptor = NULL;
  ipmi_locate_info_t *pinfo = NULL;
  ipmi_locate_info_t *rv = NULL;
  extern int errno;

  if (!IPMI_INTERFACE_TYPE_VALID(type))
    {
      errno = EINVAL;
      return NULL;
    }

  if (!(pinfo = (ipmi_locate_info_t *)malloc(sizeof(struct ipmi_locate_info))))
    goto cleanup;
  memset(pinfo, '\0', sizeof(struct ipmi_locate_info));
  pinfo->interface_type = type;
  if (type == IPMI_INTERFACE_SSIF)
    pinfo->bmc_i2c_dev_name = strdup (IPMI_DEFAULT_I2C_DEVICE);

  pinfo->locate_driver_type = IPMI_LOCATE_DRIVER_ACPI;

  FIID_OBJ_CREATE_CLEANUP (obj_acpi_table_hdr, tmpl_acpi_table_hdr);

  FIID_OBJ_CREATE_CLEANUP (obj_acpi_spmi_table_descriptor, tmpl_acpi_spmi_table_descriptor);

  if (ipmi_acpi_get_spmi_table (type,
				obj_acpi_table_hdr,
				obj_acpi_spmi_table_descriptor) != 0)
    goto cleanup;
  
  /* I don't see any reason to perform this check now -- Anand Babu */
  /* This field must always be 01h to be compatible with any software
     that implements previous versions of this spec. */
  /*
    {
    uint64_t ipmi_legacy;
    fiid_obj_get (obj_acpi_spmi_table_descriptor, tmpl_acpi_spmi_table_descriptor, (uint8_t *)"ipmi_legacy", &ipmi_legacy);
    if (ipmi_legacy != 1)
    {
    errno = ENODEV;
    return (NULL);
    }
    }
  */

  /* IPMI version */
  {
    uint64_t ipmi_ver_maj, ipmi_ver_min;

    FIID_OBJ_GET_CLEANUP (obj_acpi_spmi_table_descriptor, 
			  (uint8_t *)"specification_revision.major", 
			  &ipmi_ver_maj);

    FIID_OBJ_GET_CLEANUP (obj_acpi_spmi_table_descriptor,
			  (uint8_t *)"specification_revision.minor", 
			  &ipmi_ver_min);

    pinfo->ipmi_ver_major = ipmi_ver_maj;
    pinfo->ipmi_ver_minor = ipmi_ver_min;
  }  

  /* Interface type - KCS, SMIC, SSIF, BT */
  {
    uint64_t interface_type;

    FIID_OBJ_GET_CLEANUP (obj_acpi_spmi_table_descriptor, 
			  (uint8_t *)"interface_type", 
			  &interface_type);
    
    if (!IPMI_INTERFACE_TYPE_VALID(interface_type))
      {
	errno = ENODEV;
	goto cleanup;
      }

    pinfo->interface_type = interface_type;
  }
  
  /* Address space id (memory mapped, IO mapped, SMBus) and IO base address */
  {
    uint64_t addr_space_id;
    uint64_t base_addr;

    FIID_OBJ_GET_CLEANUP (obj_acpi_spmi_table_descriptor, 
			  (uint8_t *)"base_address.address_space_id", 
			  &addr_space_id);

    FIID_OBJ_GET_CLEANUP (obj_acpi_spmi_table_descriptor, 
			  (uint8_t *)"base_address.address", 
			  &base_addr);

    switch (addr_space_id)
      {
      case IPMI_ACPI_ADDRESS_SPACE_ID_SYSTEM_MEMORY:
	{
	  pinfo->addr_space_id = IPMI_ADDRESS_SPACE_ID_SYSTEM_MEMORY;
	  pinfo->base_addr.bmc_iobase_addr = base_addr;
	  break;
	}
      case IPMI_ACPI_ADDRESS_SPACE_ID_SYSTEM_IO:
	{
	  pinfo->addr_space_id = IPMI_ADDRESS_SPACE_ID_SYSTEM_IO;
	  pinfo->base_addr.bmc_membase_addr = base_addr;
	  break;
	}
      case IPMI_ACPI_ADDRESS_SPACE_ID_SMBUS:
	{
	  pinfo->addr_space_id = IPMI_ADDRESS_SPACE_ID_SMBUS;
	  pinfo->base_addr.bmc_smbus_slave_addr = base_addr;
	  break;
	}
      default:
	{
	  errno = ENODEV;
	  goto cleanup;
	}
      }
  }
  
  /* Register spacing */
  {
    uint64_t reg_bit_width;

    FIID_OBJ_GET_CLEANUP (obj_acpi_spmi_table_descriptor, 
			  (uint8_t *)"base_address.register_bit_width", 
			  &reg_bit_width);
    pinfo->reg_space = (reg_bit_width / 8);
  }

  rv = pinfo;
 cleanup:
  FIID_OBJ_DESTROY_NO_RETURN (obj_acpi_table_hdr);
  FIID_OBJ_DESTROY_NO_RETURN (obj_acpi_spmi_table_descriptor);
  if (!rv)
    ipmi_locate_destroy(pinfo);
  return (rv);
}

