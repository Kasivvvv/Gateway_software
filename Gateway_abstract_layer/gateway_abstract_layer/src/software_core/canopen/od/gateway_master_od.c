#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if !LELY_NO_MALLOC
#error Static object dictionaries are only supported when dynamic memory allocation is disabled.
#endif

#include <lely/co/detail/dev.h>
#include <lely/co/detail/obj.h>
#include <lely/util/cmp.h>

#define CO_DEV_STRING(s) NULL

static co_dev_t gateway_master_od = {
	.netid = 0,
	.id = 127,
	.tree = { .cmp = &uint16_cmp },
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("GatewayMaster"),
	.vendor_name = CO_DEV_STRING("Gateway"),
#endif
	.vendor_id = 0x00000000,
#if !LELY_NO_CO_OBJ_NAME
	.product_name = CO_DEV_STRING("EtherCAT-CAN Gateway master"),
#endif
	.product_code = 0x00000002,
	.revision = 0x00000001,
#if !LELY_NO_CO_OBJ_NAME
	.order_code = CO_DEV_STRING(""),
#endif
	.baud = 0 | CO_BAUD_1000,
	.rate = 1000,
	.lss = 0,
	.dummy = 0x000000FC
};

static struct {
	co_unsigned32_t sub0;
} gateway_master_od_1000_val = {
	.sub0 = 0x00000000,
};

static co_obj_t gateway_master_od_1000 = {
	.node = { .key = &gateway_master_od_1000.idx },
	.idx = 0x1000,
	.code = CO_OBJECT_VAR,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Device type"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1000_val,
	.size = sizeof(gateway_master_od_1000_val)
};

static co_sub_t gateway_master_od_1000sub0 = {
	.node = { .key = &gateway_master_od_1000sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Device type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000000 },
#endif
	.val = &gateway_master_od_1000_val.sub0,
	.access = CO_ACCESS_RO,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
} gateway_master_od_1001_val = {
	.sub0 = 0x00,
};

static co_obj_t gateway_master_od_1001 = {
	.node = { .key = &gateway_master_od_1001.idx },
	.idx = 0x1001,
	.code = CO_OBJECT_VAR,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Error register"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1001_val,
	.size = sizeof(gateway_master_od_1001_val)
};

static co_sub_t gateway_master_od_1001sub0 = {
	.node = { .key = &gateway_master_od_1001sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Error register"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x00 },
#endif
	.val = &gateway_master_od_1001_val.sub0,
	.access = CO_ACCESS_RO,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned32_t sub0;
} gateway_master_od_1005_val = {
	.sub0 = 0x40000080,
};

static co_obj_t gateway_master_od_1005 = {
	.node = { .key = &gateway_master_od_1005.idx },
	.idx = 0x1005,
	.code = CO_OBJECT_VAR,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID SYNC message"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1005_val,
	.size = sizeof(gateway_master_od_1005_val)
};

static co_sub_t gateway_master_od_1005sub0 = {
	.node = { .key = &gateway_master_od_1005sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID SYNC message"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x40000080 },
#endif
	.val = &gateway_master_od_1005_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned32_t sub0;
} gateway_master_od_1006_val = {
	.sub0 = 0x00000000,
};

static co_obj_t gateway_master_od_1006 = {
	.node = { .key = &gateway_master_od_1006.idx },
	.idx = 0x1006,
	.code = CO_OBJECT_VAR,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Communication cycle period"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1006_val,
	.size = sizeof(gateway_master_od_1006_val)
};

static co_sub_t gateway_master_od_1006sub0 = {
	.node = { .key = &gateway_master_od_1006sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Communication cycle period"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000000 },
#endif
	.val = &gateway_master_od_1006_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned32_t sub2;
	co_unsigned32_t sub3;
	co_unsigned32_t sub4;
	co_unsigned32_t sub5;
	co_unsigned32_t sub6;
	co_unsigned32_t sub7;
} gateway_master_od_1016_val = {
	.sub0 = 0x07,
	.sub1 = 0x000100FA,
	.sub2 = 0x000200FA,
	.sub3 = 0x000300FA,
	.sub4 = 0x000400FA,
	.sub5 = 0x000500FA,
	.sub6 = 0x000600FA,
	.sub7 = 0x000700FA,
};

static co_obj_t gateway_master_od_1016 = {
	.node = { .key = &gateway_master_od_1016.idx },
	.idx = 0x1016,
	.code = CO_OBJECT_ARRAY,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Consumer heartbeat time"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1016_val,
	.size = sizeof(gateway_master_od_1016_val)
};

static co_sub_t gateway_master_od_1016sub0 = {
	.node = { .key = &gateway_master_od_1016sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("NrOfObjects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x07 },
#endif
	.val = &gateway_master_od_1016_val.sub0,
	.access = CO_ACCESS_RO,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1016sub1 = {
	.node = { .key = &gateway_master_od_1016sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Consumer heartbeat time1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000000 },
#endif
	.val = &gateway_master_od_1016_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1016sub2 = {
	.node = { .key = &gateway_master_od_1016sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Consumer heartbeat time2"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000000 },
#endif
	.val = &gateway_master_od_1016_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1016sub3 = {
	.node = { .key = &gateway_master_od_1016sub3.subidx },
	.subidx = 0x03,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Consumer heartbeat time3"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000000 },
#endif
	.val = &gateway_master_od_1016_val.sub3,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1016sub4 = {
	.node = { .key = &gateway_master_od_1016sub4.subidx },
	.subidx = 0x04,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Consumer heartbeat time4"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000000 },
#endif
	.val = &gateway_master_od_1016_val.sub4,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1016sub5 = {
	.node = { .key = &gateway_master_od_1016sub5.subidx },
	.subidx = 0x05,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Consumer heartbeat time5"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000000 },
#endif
	.val = &gateway_master_od_1016_val.sub5,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1016sub6 = {
	.node = { .key = &gateway_master_od_1016sub6.subidx },
	.subidx = 0x06,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Consumer heartbeat time6"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000000 },
#endif
	.val = &gateway_master_od_1016_val.sub6,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1016sub7 = {
	.node = { .key = &gateway_master_od_1016sub7.subidx },
	.subidx = 0x07,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Consumer heartbeat time7"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000000 },
#endif
	.val = &gateway_master_od_1016_val.sub7,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned16_t sub0;
} gateway_master_od_1017_val = {
	.sub0 = 0x0064,
};

static co_obj_t gateway_master_od_1017 = {
	.node = { .key = &gateway_master_od_1017.idx },
	.idx = 0x1017,
	.code = CO_OBJECT_VAR,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Producer heartbeat time"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1017_val,
	.size = sizeof(gateway_master_od_1017_val)
};

static co_sub_t gateway_master_od_1017sub0 = {
	.node = { .key = &gateway_master_od_1017sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED16,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Producer heartbeat time"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u16 = CO_UNSIGNED16_MIN },
	.max = { .u16 = CO_UNSIGNED16_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u16 = 0x0064 },
#endif
	.val = &gateway_master_od_1017_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned32_t sub2;
	co_unsigned32_t sub3;
	co_unsigned32_t sub4;
} gateway_master_od_1018_val = {
	.sub0 = 0x04,
	.sub1 = 0x00000000,
	.sub2 = 0x00000002,
	.sub3 = 0x00000001,
	.sub4 = 0x00000001,
};

static co_obj_t gateway_master_od_1018 = {
	.node = { .key = &gateway_master_od_1018.idx },
	.idx = 0x1018,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Identity object"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1018_val,
	.size = sizeof(gateway_master_od_1018_val)
};

static co_sub_t gateway_master_od_1018sub0 = {
	.node = { .key = &gateway_master_od_1018sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x04 },
#endif
	.val = &gateway_master_od_1018_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1018sub1 = {
	.node = { .key = &gateway_master_od_1018sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Vendor-ID"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000000 },
#endif
	.val = &gateway_master_od_1018_val.sub1,
	.access = CO_ACCESS_RO,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1018sub2 = {
	.node = { .key = &gateway_master_od_1018sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Product code"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000002 },
#endif
	.val = &gateway_master_od_1018_val.sub2,
	.access = CO_ACCESS_RO,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1018sub3 = {
	.node = { .key = &gateway_master_od_1018sub3.subidx },
	.subidx = 0x03,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Revision number"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000001 },
#endif
	.val = &gateway_master_od_1018_val.sub3,
	.access = CO_ACCESS_RO,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1018sub4 = {
	.node = { .key = &gateway_master_od_1018sub4.subidx },
	.subidx = 0x04,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Serial number"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000001 },
#endif
	.val = &gateway_master_od_1018_val.sub4,
	.access = CO_ACCESS_RO,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1400_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000181,
	.sub2 = CO_UNSIGNED8_MAX,
};

static co_obj_t gateway_master_od_1400 = {
	.node = { .key = &gateway_master_od_1400.idx },
	.idx = 0x1400,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 1 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1400_val,
	.size = sizeof(gateway_master_od_1400_val)
};

static co_sub_t gateway_master_od_1400sub0 = {
	.node = { .key = &gateway_master_od_1400sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1400_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1400sub1 = {
	.node = { .key = &gateway_master_od_1400sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by RPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000181 },
#endif
	.val = &gateway_master_od_1400_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1400sub2 = {
	.node = { .key = &gateway_master_od_1400sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = CO_UNSIGNED8_MAX },
#endif
	.val = &gateway_master_od_1400_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1401_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000182,
	.sub2 = CO_UNSIGNED8_MAX,
};

static co_obj_t gateway_master_od_1401 = {
	.node = { .key = &gateway_master_od_1401.idx },
	.idx = 0x1401,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 2 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1401_val,
	.size = sizeof(gateway_master_od_1401_val)
};

static co_sub_t gateway_master_od_1401sub0 = {
	.node = { .key = &gateway_master_od_1401sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1401_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1401sub1 = {
	.node = { .key = &gateway_master_od_1401sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by RPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000182 },
#endif
	.val = &gateway_master_od_1401_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1401sub2 = {
	.node = { .key = &gateway_master_od_1401sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = CO_UNSIGNED8_MAX },
#endif
	.val = &gateway_master_od_1401_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1402_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000183,
	.sub2 = CO_UNSIGNED8_MAX,
};

static co_obj_t gateway_master_od_1402 = {
	.node = { .key = &gateway_master_od_1402.idx },
	.idx = 0x1402,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 3 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1402_val,
	.size = sizeof(gateway_master_od_1402_val)
};

static co_sub_t gateway_master_od_1402sub0 = {
	.node = { .key = &gateway_master_od_1402sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1402_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1402sub1 = {
	.node = { .key = &gateway_master_od_1402sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by RPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000183 },
#endif
	.val = &gateway_master_od_1402_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1402sub2 = {
	.node = { .key = &gateway_master_od_1402sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = CO_UNSIGNED8_MAX },
#endif
	.val = &gateway_master_od_1402_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1403_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000184,
	.sub2 = CO_UNSIGNED8_MAX,
};

static co_obj_t gateway_master_od_1403 = {
	.node = { .key = &gateway_master_od_1403.idx },
	.idx = 0x1403,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 4 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1403_val,
	.size = sizeof(gateway_master_od_1403_val)
};

static co_sub_t gateway_master_od_1403sub0 = {
	.node = { .key = &gateway_master_od_1403sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1403_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1403sub1 = {
	.node = { .key = &gateway_master_od_1403sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by RPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000184 },
#endif
	.val = &gateway_master_od_1403_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1403sub2 = {
	.node = { .key = &gateway_master_od_1403sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = CO_UNSIGNED8_MAX },
#endif
	.val = &gateway_master_od_1403_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1404_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000185,
	.sub2 = CO_UNSIGNED8_MAX,
};

static co_obj_t gateway_master_od_1404 = {
	.node = { .key = &gateway_master_od_1404.idx },
	.idx = 0x1404,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 5 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1404_val,
	.size = sizeof(gateway_master_od_1404_val)
};

static co_sub_t gateway_master_od_1404sub0 = {
	.node = { .key = &gateway_master_od_1404sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1404_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1404sub1 = {
	.node = { .key = &gateway_master_od_1404sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by RPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000185 },
#endif
	.val = &gateway_master_od_1404_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1404sub2 = {
	.node = { .key = &gateway_master_od_1404sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = CO_UNSIGNED8_MAX },
#endif
	.val = &gateway_master_od_1404_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1405_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000186,
	.sub2 = CO_UNSIGNED8_MAX,
};

static co_obj_t gateway_master_od_1405 = {
	.node = { .key = &gateway_master_od_1405.idx },
	.idx = 0x1405,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 6 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1405_val,
	.size = sizeof(gateway_master_od_1405_val)
};

static co_sub_t gateway_master_od_1405sub0 = {
	.node = { .key = &gateway_master_od_1405sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1405_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1405sub1 = {
	.node = { .key = &gateway_master_od_1405sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by RPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000186 },
#endif
	.val = &gateway_master_od_1405_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1405sub2 = {
	.node = { .key = &gateway_master_od_1405sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = CO_UNSIGNED8_MAX },
#endif
	.val = &gateway_master_od_1405_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1406_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000187,
	.sub2 = CO_UNSIGNED8_MAX,
};

static co_obj_t gateway_master_od_1406 = {
	.node = { .key = &gateway_master_od_1406.idx },
	.idx = 0x1406,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 7 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1406_val,
	.size = sizeof(gateway_master_od_1406_val)
};

static co_sub_t gateway_master_od_1406sub0 = {
	.node = { .key = &gateway_master_od_1406sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1406_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1406sub1 = {
	.node = { .key = &gateway_master_od_1406sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by RPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000187 },
#endif
	.val = &gateway_master_od_1406_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1406sub2 = {
	.node = { .key = &gateway_master_od_1406sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = CO_UNSIGNED8_MAX },
#endif
	.val = &gateway_master_od_1406_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1600_val = {
	.sub0 = 0x01,
	.sub1 = 0x22000140,
};

static co_obj_t gateway_master_od_1600 = {
	.node = { .key = &gateway_master_od_1600.idx },
	.idx = 0x1600,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 1 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1600_val,
	.size = sizeof(gateway_master_od_1600_val)
};

static co_sub_t gateway_master_od_1600sub0 = {
	.node = { .key = &gateway_master_od_1600sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1600_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1600sub1 = {
	.node = { .key = &gateway_master_od_1600sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x22000140 },
#endif
	.val = &gateway_master_od_1600_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1601_val = {
	.sub0 = 0x01,
	.sub1 = 0x22000240,
};

static co_obj_t gateway_master_od_1601 = {
	.node = { .key = &gateway_master_od_1601.idx },
	.idx = 0x1601,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 2 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1601_val,
	.size = sizeof(gateway_master_od_1601_val)
};

static co_sub_t gateway_master_od_1601sub0 = {
	.node = { .key = &gateway_master_od_1601sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1601_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1601sub1 = {
	.node = { .key = &gateway_master_od_1601sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x22000240 },
#endif
	.val = &gateway_master_od_1601_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1602_val = {
	.sub0 = 0x01,
	.sub1 = 0x22000340,
};

static co_obj_t gateway_master_od_1602 = {
	.node = { .key = &gateway_master_od_1602.idx },
	.idx = 0x1602,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 3 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1602_val,
	.size = sizeof(gateway_master_od_1602_val)
};

static co_sub_t gateway_master_od_1602sub0 = {
	.node = { .key = &gateway_master_od_1602sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1602_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1602sub1 = {
	.node = { .key = &gateway_master_od_1602sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x22000340 },
#endif
	.val = &gateway_master_od_1602_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1603_val = {
	.sub0 = 0x01,
	.sub1 = 0x22000440,
};

static co_obj_t gateway_master_od_1603 = {
	.node = { .key = &gateway_master_od_1603.idx },
	.idx = 0x1603,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 4 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1603_val,
	.size = sizeof(gateway_master_od_1603_val)
};

static co_sub_t gateway_master_od_1603sub0 = {
	.node = { .key = &gateway_master_od_1603sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1603_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1603sub1 = {
	.node = { .key = &gateway_master_od_1603sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x22000440 },
#endif
	.val = &gateway_master_od_1603_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1604_val = {
	.sub0 = 0x01,
	.sub1 = 0x22000540,
};

static co_obj_t gateway_master_od_1604 = {
	.node = { .key = &gateway_master_od_1604.idx },
	.idx = 0x1604,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 5 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1604_val,
	.size = sizeof(gateway_master_od_1604_val)
};

static co_sub_t gateway_master_od_1604sub0 = {
	.node = { .key = &gateway_master_od_1604sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1604_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1604sub1 = {
	.node = { .key = &gateway_master_od_1604sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x22000540 },
#endif
	.val = &gateway_master_od_1604_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1605_val = {
	.sub0 = 0x01,
	.sub1 = 0x22000640,
};

static co_obj_t gateway_master_od_1605 = {
	.node = { .key = &gateway_master_od_1605.idx },
	.idx = 0x1605,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 6 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1605_val,
	.size = sizeof(gateway_master_od_1605_val)
};

static co_sub_t gateway_master_od_1605sub0 = {
	.node = { .key = &gateway_master_od_1605sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1605_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1605sub1 = {
	.node = { .key = &gateway_master_od_1605sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x22000640 },
#endif
	.val = &gateway_master_od_1605_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1606_val = {
	.sub0 = 0x01,
	.sub1 = 0x22000740,
};

static co_obj_t gateway_master_od_1606 = {
	.node = { .key = &gateway_master_od_1606.idx },
	.idx = 0x1606,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("RPDO 7 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1606_val,
	.size = sizeof(gateway_master_od_1606_val)
};

static co_sub_t gateway_master_od_1606sub0 = {
	.node = { .key = &gateway_master_od_1606sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1606_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1606sub1 = {
	.node = { .key = &gateway_master_od_1606sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x22000740 },
#endif
	.val = &gateway_master_od_1606_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1800_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000201,
	.sub2 = 0x01,
};

static co_obj_t gateway_master_od_1800 = {
	.node = { .key = &gateway_master_od_1800.idx },
	.idx = 0x1800,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 1 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1800_val,
	.size = sizeof(gateway_master_od_1800_val)
};

static co_sub_t gateway_master_od_1800sub0 = {
	.node = { .key = &gateway_master_od_1800sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1800_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1800sub1 = {
	.node = { .key = &gateway_master_od_1800sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by TPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000201 },
#endif
	.val = &gateway_master_od_1800_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1800sub2 = {
	.node = { .key = &gateway_master_od_1800sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1800_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1801_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000202,
	.sub2 = 0x01,
};

static co_obj_t gateway_master_od_1801 = {
	.node = { .key = &gateway_master_od_1801.idx },
	.idx = 0x1801,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 2 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1801_val,
	.size = sizeof(gateway_master_od_1801_val)
};

static co_sub_t gateway_master_od_1801sub0 = {
	.node = { .key = &gateway_master_od_1801sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1801_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1801sub1 = {
	.node = { .key = &gateway_master_od_1801sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by TPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000202 },
#endif
	.val = &gateway_master_od_1801_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1801sub2 = {
	.node = { .key = &gateway_master_od_1801sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1801_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1802_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000203,
	.sub2 = 0x01,
};

static co_obj_t gateway_master_od_1802 = {
	.node = { .key = &gateway_master_od_1802.idx },
	.idx = 0x1802,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 3 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1802_val,
	.size = sizeof(gateway_master_od_1802_val)
};

static co_sub_t gateway_master_od_1802sub0 = {
	.node = { .key = &gateway_master_od_1802sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1802_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1802sub1 = {
	.node = { .key = &gateway_master_od_1802sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by TPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000203 },
#endif
	.val = &gateway_master_od_1802_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1802sub2 = {
	.node = { .key = &gateway_master_od_1802sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1802_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1803_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000204,
	.sub2 = 0x01,
};

static co_obj_t gateway_master_od_1803 = {
	.node = { .key = &gateway_master_od_1803.idx },
	.idx = 0x1803,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 4 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1803_val,
	.size = sizeof(gateway_master_od_1803_val)
};

static co_sub_t gateway_master_od_1803sub0 = {
	.node = { .key = &gateway_master_od_1803sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1803_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1803sub1 = {
	.node = { .key = &gateway_master_od_1803sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by TPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000204 },
#endif
	.val = &gateway_master_od_1803_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1803sub2 = {
	.node = { .key = &gateway_master_od_1803sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1803_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1804_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000205,
	.sub2 = 0x01,
};

static co_obj_t gateway_master_od_1804 = {
	.node = { .key = &gateway_master_od_1804.idx },
	.idx = 0x1804,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 5 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1804_val,
	.size = sizeof(gateway_master_od_1804_val)
};

static co_sub_t gateway_master_od_1804sub0 = {
	.node = { .key = &gateway_master_od_1804sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1804_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1804sub1 = {
	.node = { .key = &gateway_master_od_1804sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by TPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000205 },
#endif
	.val = &gateway_master_od_1804_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1804sub2 = {
	.node = { .key = &gateway_master_od_1804sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1804_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1805_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000206,
	.sub2 = 0x01,
};

static co_obj_t gateway_master_od_1805 = {
	.node = { .key = &gateway_master_od_1805.idx },
	.idx = 0x1805,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 6 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1805_val,
	.size = sizeof(gateway_master_od_1805_val)
};

static co_sub_t gateway_master_od_1805sub0 = {
	.node = { .key = &gateway_master_od_1805sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1805_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1805sub1 = {
	.node = { .key = &gateway_master_od_1805sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by TPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000206 },
#endif
	.val = &gateway_master_od_1805_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1805sub2 = {
	.node = { .key = &gateway_master_od_1805sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1805_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
	co_unsigned8_t sub2;
} gateway_master_od_1806_val = {
	.sub0 = 0x02,
	.sub1 = 0x00000207,
	.sub2 = 0x01,
};

static co_obj_t gateway_master_od_1806 = {
	.node = { .key = &gateway_master_od_1806.idx },
	.idx = 0x1806,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 7 communication parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1806_val,
	.size = sizeof(gateway_master_od_1806_val)
};

static co_sub_t gateway_master_od_1806sub0 = {
	.node = { .key = &gateway_master_od_1806sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x02 },
#endif
	.val = &gateway_master_od_1806_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1806sub1 = {
	.node = { .key = &gateway_master_od_1806sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("COB-ID used by TPDO"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000207 },
#endif
	.val = &gateway_master_od_1806_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1806sub2 = {
	.node = { .key = &gateway_master_od_1806sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Transmission type"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1806_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1A00_val = {
	.sub0 = 0x01,
	.sub1 = 0x23000140,
};

static co_obj_t gateway_master_od_1A00 = {
	.node = { .key = &gateway_master_od_1A00.idx },
	.idx = 0x1A00,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 1 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1A00_val,
	.size = sizeof(gateway_master_od_1A00_val)
};

static co_sub_t gateway_master_od_1A00sub0 = {
	.node = { .key = &gateway_master_od_1A00sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1A00_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1A00sub1 = {
	.node = { .key = &gateway_master_od_1A00sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x23000140 },
#endif
	.val = &gateway_master_od_1A00_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1A01_val = {
	.sub0 = 0x01,
	.sub1 = 0x23000240,
};

static co_obj_t gateway_master_od_1A01 = {
	.node = { .key = &gateway_master_od_1A01.idx },
	.idx = 0x1A01,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 2 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1A01_val,
	.size = sizeof(gateway_master_od_1A01_val)
};

static co_sub_t gateway_master_od_1A01sub0 = {
	.node = { .key = &gateway_master_od_1A01sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1A01_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1A01sub1 = {
	.node = { .key = &gateway_master_od_1A01sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x23000240 },
#endif
	.val = &gateway_master_od_1A01_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1A02_val = {
	.sub0 = 0x01,
	.sub1 = 0x23000340,
};

static co_obj_t gateway_master_od_1A02 = {
	.node = { .key = &gateway_master_od_1A02.idx },
	.idx = 0x1A02,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 3 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1A02_val,
	.size = sizeof(gateway_master_od_1A02_val)
};

static co_sub_t gateway_master_od_1A02sub0 = {
	.node = { .key = &gateway_master_od_1A02sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1A02_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1A02sub1 = {
	.node = { .key = &gateway_master_od_1A02sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x23000340 },
#endif
	.val = &gateway_master_od_1A02_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1A03_val = {
	.sub0 = 0x01,
	.sub1 = 0x23000440,
};

static co_obj_t gateway_master_od_1A03 = {
	.node = { .key = &gateway_master_od_1A03.idx },
	.idx = 0x1A03,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 4 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1A03_val,
	.size = sizeof(gateway_master_od_1A03_val)
};

static co_sub_t gateway_master_od_1A03sub0 = {
	.node = { .key = &gateway_master_od_1A03sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1A03_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1A03sub1 = {
	.node = { .key = &gateway_master_od_1A03sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x23000440 },
#endif
	.val = &gateway_master_od_1A03_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1A04_val = {
	.sub0 = 0x01,
	.sub1 = 0x23000540,
};

static co_obj_t gateway_master_od_1A04 = {
	.node = { .key = &gateway_master_od_1A04.idx },
	.idx = 0x1A04,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 5 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1A04_val,
	.size = sizeof(gateway_master_od_1A04_val)
};

static co_sub_t gateway_master_od_1A04sub0 = {
	.node = { .key = &gateway_master_od_1A04sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1A04_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1A04sub1 = {
	.node = { .key = &gateway_master_od_1A04sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x23000540 },
#endif
	.val = &gateway_master_od_1A04_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1A05_val = {
	.sub0 = 0x01,
	.sub1 = 0x23000640,
};

static co_obj_t gateway_master_od_1A05 = {
	.node = { .key = &gateway_master_od_1A05.idx },
	.idx = 0x1A05,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 6 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1A05_val,
	.size = sizeof(gateway_master_od_1A05_val)
};

static co_sub_t gateway_master_od_1A05sub0 = {
	.node = { .key = &gateway_master_od_1A05sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1A05_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1A05sub1 = {
	.node = { .key = &gateway_master_od_1A05sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x23000640 },
#endif
	.val = &gateway_master_od_1A05_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned32_t sub1;
} gateway_master_od_1A06_val = {
	.sub0 = 0x01,
	.sub1 = 0x23000740,
};

static co_obj_t gateway_master_od_1A06 = {
	.node = { .key = &gateway_master_od_1A06.idx },
	.idx = 0x1A06,
	.code = CO_OBJECT_RECORD,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("TPDO 7 mapping parameter"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1A06_val,
	.size = sizeof(gateway_master_od_1A06_val)
};

static co_sub_t gateway_master_od_1A06sub0 = {
	.node = { .key = &gateway_master_od_1A06sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Number of mapped objects"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x01 },
#endif
	.val = &gateway_master_od_1A06_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_1A06sub1 = {
	.node = { .key = &gateway_master_od_1A06sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Mapped object 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x23000740 },
#endif
	.val = &gateway_master_od_1A06_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned32_t sub0;
} gateway_master_od_1F80_val = {
	.sub0 = 0x00000005,
};

static co_obj_t gateway_master_od_1F80 = {
	.node = { .key = &gateway_master_od_1F80.idx },
	.idx = 0x1F80,
	.code = CO_OBJECT_VAR,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("NMT startup"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_1F80_val,
	.size = sizeof(gateway_master_od_1F80_val)
};

static co_sub_t gateway_master_od_1F80sub0 = {
	.node = { .key = &gateway_master_od_1F80sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED32,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("NMT startup"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u32 = CO_UNSIGNED32_MIN },
	.max = { .u32 = CO_UNSIGNED32_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u32 = 0x00000005 },
#endif
	.val = &gateway_master_od_1F80_val.sub0,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned64_t sub1;
	co_unsigned64_t sub2;
	co_unsigned64_t sub3;
	co_unsigned64_t sub4;
	co_unsigned64_t sub5;
	co_unsigned64_t sub6;
	co_unsigned64_t sub7;
} gateway_master_od_2200_val = {
	.sub0 = 0x07,
	.sub1 = 0x0000000000000000,
	.sub2 = 0x0000000000000000,
	.sub3 = 0x0000000000000000,
	.sub4 = 0x0000000000000000,
	.sub5 = 0x0000000000000000,
	.sub6 = 0x0000000000000000,
	.sub7 = 0x0000000000000000,
};

static co_obj_t gateway_master_od_2200 = {
	.node = { .key = &gateway_master_od_2200.idx },
	.idx = 0x2200,
	.code = CO_OBJECT_ARRAY,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Feedback of node (TPDO of node n, to EtherCAT)"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_2200_val,
	.size = sizeof(gateway_master_od_2200_val)
};

static co_sub_t gateway_master_od_2200sub0 = {
	.node = { .key = &gateway_master_od_2200sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x07 },
#endif
	.val = &gateway_master_od_2200_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2200sub1 = {
	.node = { .key = &gateway_master_od_2200sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2200_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2200sub2 = {
	.node = { .key = &gateway_master_od_2200sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 2"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2200_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2200sub3 = {
	.node = { .key = &gateway_master_od_2200sub3.subidx },
	.subidx = 0x03,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 3"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2200_val.sub3,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2200sub4 = {
	.node = { .key = &gateway_master_od_2200sub4.subidx },
	.subidx = 0x04,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 4"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2200_val.sub4,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2200sub5 = {
	.node = { .key = &gateway_master_od_2200sub5.subidx },
	.subidx = 0x05,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 5"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2200_val.sub5,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2200sub6 = {
	.node = { .key = &gateway_master_od_2200sub6.subidx },
	.subidx = 0x06,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 6"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2200_val.sub6,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2200sub7 = {
	.node = { .key = &gateway_master_od_2200sub7.subidx },
	.subidx = 0x07,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 7"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2200_val.sub7,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static struct {
	co_unsigned8_t sub0;
	co_unsigned64_t sub1;
	co_unsigned64_t sub2;
	co_unsigned64_t sub3;
	co_unsigned64_t sub4;
	co_unsigned64_t sub5;
	co_unsigned64_t sub6;
	co_unsigned64_t sub7;
} gateway_master_od_2300_val = {
	.sub0 = 0x07,
	.sub1 = 0x0000000000000000,
	.sub2 = 0x0000000000000000,
	.sub3 = 0x0000000000000000,
	.sub4 = 0x0000000000000000,
	.sub5 = 0x0000000000000000,
	.sub6 = 0x0000000000000000,
	.sub7 = 0x0000000000000000,
};

static co_obj_t gateway_master_od_2300 = {
	.node = { .key = &gateway_master_od_2300.idx },
	.idx = 0x2300,
	.code = CO_OBJECT_ARRAY,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Set-point of node (RPDO of node n, from EtherCAT)"),
#endif
	.tree = { .cmp = &uint8_cmp },
	.val = &gateway_master_od_2300_val,
	.size = sizeof(gateway_master_od_2300_val)
};

static co_sub_t gateway_master_od_2300sub0 = {
	.node = { .key = &gateway_master_od_2300sub0.subidx },
	.subidx = 0x00,
	.type = CO_DEFTYPE_UNSIGNED8,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Highest sub-index supported"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u8 = CO_UNSIGNED8_MIN },
	.max = { .u8 = CO_UNSIGNED8_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u8 = 0x07 },
#endif
	.val = &gateway_master_od_2300_val.sub0,
	.access = CO_ACCESS_CONST,
	.pdo_mapping = 0,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2300sub1 = {
	.node = { .key = &gateway_master_od_2300sub1.subidx },
	.subidx = 0x01,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 1"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2300_val.sub1,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2300sub2 = {
	.node = { .key = &gateway_master_od_2300sub2.subidx },
	.subidx = 0x02,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 2"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2300_val.sub2,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2300sub3 = {
	.node = { .key = &gateway_master_od_2300sub3.subidx },
	.subidx = 0x03,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 3"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2300_val.sub3,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2300sub4 = {
	.node = { .key = &gateway_master_od_2300sub4.subidx },
	.subidx = 0x04,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 4"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2300_val.sub4,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2300sub5 = {
	.node = { .key = &gateway_master_od_2300sub5.subidx },
	.subidx = 0x05,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 5"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2300_val.sub5,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2300sub6 = {
	.node = { .key = &gateway_master_od_2300sub6.subidx },
	.subidx = 0x06,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 6"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2300_val.sub6,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

static co_sub_t gateway_master_od_2300sub7 = {
	.node = { .key = &gateway_master_od_2300sub7.subidx },
	.subidx = 0x07,
	.type = CO_DEFTYPE_UNSIGNED64,
#if !LELY_NO_CO_OBJ_NAME
	.name = CO_DEV_STRING("Node 7"),
#endif
#if !LELY_NO_CO_OBJ_LIMITS
	.min = { .u64 = CO_UNSIGNED64_MIN },
	.max = { .u64 = CO_UNSIGNED64_MAX },
#endif
#if !LELY_NO_CO_OBJ_DEFAULT
	.def = { .u64 = 0x0000000000000000 },
#endif
	.val = &gateway_master_od_2300_val.sub7,
	.access = CO_ACCESS_RW,
	.pdo_mapping = 1,
	.flags = 0,
	.dn_ind = &co_sub_default_dn_ind,
#if !LELY_NO_CO_OBJ_UPLOAD
	.up_ind = &co_sub_default_up_ind
#endif
};

// suppress missing-prototype warning
co_dev_t * gateway_master_od_init(void);
co_dev_t *
gateway_master_od_init(void) {
	static co_dev_t *dev = NULL;
	if (!dev) {
		dev = &gateway_master_od;

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1000);
		co_obj_insert_sub(&gateway_master_od_1000, &gateway_master_od_1000sub0);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1001);
		co_obj_insert_sub(&gateway_master_od_1001, &gateway_master_od_1001sub0);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1005);
		co_obj_insert_sub(&gateway_master_od_1005, &gateway_master_od_1005sub0);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1006);
		co_obj_insert_sub(&gateway_master_od_1006, &gateway_master_od_1006sub0);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1016);
		co_obj_insert_sub(&gateway_master_od_1016, &gateway_master_od_1016sub0);
		co_obj_insert_sub(&gateway_master_od_1016, &gateway_master_od_1016sub1);
		co_obj_insert_sub(&gateway_master_od_1016, &gateway_master_od_1016sub2);
		co_obj_insert_sub(&gateway_master_od_1016, &gateway_master_od_1016sub3);
		co_obj_insert_sub(&gateway_master_od_1016, &gateway_master_od_1016sub4);
		co_obj_insert_sub(&gateway_master_od_1016, &gateway_master_od_1016sub5);
		co_obj_insert_sub(&gateway_master_od_1016, &gateway_master_od_1016sub6);
		co_obj_insert_sub(&gateway_master_od_1016, &gateway_master_od_1016sub7);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1017);
		co_obj_insert_sub(&gateway_master_od_1017, &gateway_master_od_1017sub0);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1018);
		co_obj_insert_sub(&gateway_master_od_1018, &gateway_master_od_1018sub0);
		co_obj_insert_sub(&gateway_master_od_1018, &gateway_master_od_1018sub1);
		co_obj_insert_sub(&gateway_master_od_1018, &gateway_master_od_1018sub2);
		co_obj_insert_sub(&gateway_master_od_1018, &gateway_master_od_1018sub3);
		co_obj_insert_sub(&gateway_master_od_1018, &gateway_master_od_1018sub4);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1400);
		co_obj_insert_sub(&gateway_master_od_1400, &gateway_master_od_1400sub0);
		co_obj_insert_sub(&gateway_master_od_1400, &gateway_master_od_1400sub1);
		co_obj_insert_sub(&gateway_master_od_1400, &gateway_master_od_1400sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1401);
		co_obj_insert_sub(&gateway_master_od_1401, &gateway_master_od_1401sub0);
		co_obj_insert_sub(&gateway_master_od_1401, &gateway_master_od_1401sub1);
		co_obj_insert_sub(&gateway_master_od_1401, &gateway_master_od_1401sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1402);
		co_obj_insert_sub(&gateway_master_od_1402, &gateway_master_od_1402sub0);
		co_obj_insert_sub(&gateway_master_od_1402, &gateway_master_od_1402sub1);
		co_obj_insert_sub(&gateway_master_od_1402, &gateway_master_od_1402sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1403);
		co_obj_insert_sub(&gateway_master_od_1403, &gateway_master_od_1403sub0);
		co_obj_insert_sub(&gateway_master_od_1403, &gateway_master_od_1403sub1);
		co_obj_insert_sub(&gateway_master_od_1403, &gateway_master_od_1403sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1404);
		co_obj_insert_sub(&gateway_master_od_1404, &gateway_master_od_1404sub0);
		co_obj_insert_sub(&gateway_master_od_1404, &gateway_master_od_1404sub1);
		co_obj_insert_sub(&gateway_master_od_1404, &gateway_master_od_1404sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1405);
		co_obj_insert_sub(&gateway_master_od_1405, &gateway_master_od_1405sub0);
		co_obj_insert_sub(&gateway_master_od_1405, &gateway_master_od_1405sub1);
		co_obj_insert_sub(&gateway_master_od_1405, &gateway_master_od_1405sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1406);
		co_obj_insert_sub(&gateway_master_od_1406, &gateway_master_od_1406sub0);
		co_obj_insert_sub(&gateway_master_od_1406, &gateway_master_od_1406sub1);
		co_obj_insert_sub(&gateway_master_od_1406, &gateway_master_od_1406sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1600);
		co_obj_insert_sub(&gateway_master_od_1600, &gateway_master_od_1600sub0);
		co_obj_insert_sub(&gateway_master_od_1600, &gateway_master_od_1600sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1601);
		co_obj_insert_sub(&gateway_master_od_1601, &gateway_master_od_1601sub0);
		co_obj_insert_sub(&gateway_master_od_1601, &gateway_master_od_1601sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1602);
		co_obj_insert_sub(&gateway_master_od_1602, &gateway_master_od_1602sub0);
		co_obj_insert_sub(&gateway_master_od_1602, &gateway_master_od_1602sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1603);
		co_obj_insert_sub(&gateway_master_od_1603, &gateway_master_od_1603sub0);
		co_obj_insert_sub(&gateway_master_od_1603, &gateway_master_od_1603sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1604);
		co_obj_insert_sub(&gateway_master_od_1604, &gateway_master_od_1604sub0);
		co_obj_insert_sub(&gateway_master_od_1604, &gateway_master_od_1604sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1605);
		co_obj_insert_sub(&gateway_master_od_1605, &gateway_master_od_1605sub0);
		co_obj_insert_sub(&gateway_master_od_1605, &gateway_master_od_1605sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1606);
		co_obj_insert_sub(&gateway_master_od_1606, &gateway_master_od_1606sub0);
		co_obj_insert_sub(&gateway_master_od_1606, &gateway_master_od_1606sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1800);
		co_obj_insert_sub(&gateway_master_od_1800, &gateway_master_od_1800sub0);
		co_obj_insert_sub(&gateway_master_od_1800, &gateway_master_od_1800sub1);
		co_obj_insert_sub(&gateway_master_od_1800, &gateway_master_od_1800sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1801);
		co_obj_insert_sub(&gateway_master_od_1801, &gateway_master_od_1801sub0);
		co_obj_insert_sub(&gateway_master_od_1801, &gateway_master_od_1801sub1);
		co_obj_insert_sub(&gateway_master_od_1801, &gateway_master_od_1801sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1802);
		co_obj_insert_sub(&gateway_master_od_1802, &gateway_master_od_1802sub0);
		co_obj_insert_sub(&gateway_master_od_1802, &gateway_master_od_1802sub1);
		co_obj_insert_sub(&gateway_master_od_1802, &gateway_master_od_1802sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1803);
		co_obj_insert_sub(&gateway_master_od_1803, &gateway_master_od_1803sub0);
		co_obj_insert_sub(&gateway_master_od_1803, &gateway_master_od_1803sub1);
		co_obj_insert_sub(&gateway_master_od_1803, &gateway_master_od_1803sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1804);
		co_obj_insert_sub(&gateway_master_od_1804, &gateway_master_od_1804sub0);
		co_obj_insert_sub(&gateway_master_od_1804, &gateway_master_od_1804sub1);
		co_obj_insert_sub(&gateway_master_od_1804, &gateway_master_od_1804sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1805);
		co_obj_insert_sub(&gateway_master_od_1805, &gateway_master_od_1805sub0);
		co_obj_insert_sub(&gateway_master_od_1805, &gateway_master_od_1805sub1);
		co_obj_insert_sub(&gateway_master_od_1805, &gateway_master_od_1805sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1806);
		co_obj_insert_sub(&gateway_master_od_1806, &gateway_master_od_1806sub0);
		co_obj_insert_sub(&gateway_master_od_1806, &gateway_master_od_1806sub1);
		co_obj_insert_sub(&gateway_master_od_1806, &gateway_master_od_1806sub2);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1A00);
		co_obj_insert_sub(&gateway_master_od_1A00, &gateway_master_od_1A00sub0);
		co_obj_insert_sub(&gateway_master_od_1A00, &gateway_master_od_1A00sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1A01);
		co_obj_insert_sub(&gateway_master_od_1A01, &gateway_master_od_1A01sub0);
		co_obj_insert_sub(&gateway_master_od_1A01, &gateway_master_od_1A01sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1A02);
		co_obj_insert_sub(&gateway_master_od_1A02, &gateway_master_od_1A02sub0);
		co_obj_insert_sub(&gateway_master_od_1A02, &gateway_master_od_1A02sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1A03);
		co_obj_insert_sub(&gateway_master_od_1A03, &gateway_master_od_1A03sub0);
		co_obj_insert_sub(&gateway_master_od_1A03, &gateway_master_od_1A03sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1A04);
		co_obj_insert_sub(&gateway_master_od_1A04, &gateway_master_od_1A04sub0);
		co_obj_insert_sub(&gateway_master_od_1A04, &gateway_master_od_1A04sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1A05);
		co_obj_insert_sub(&gateway_master_od_1A05, &gateway_master_od_1A05sub0);
		co_obj_insert_sub(&gateway_master_od_1A05, &gateway_master_od_1A05sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1A06);
		co_obj_insert_sub(&gateway_master_od_1A06, &gateway_master_od_1A06sub0);
		co_obj_insert_sub(&gateway_master_od_1A06, &gateway_master_od_1A06sub1);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_1F80);
		co_obj_insert_sub(&gateway_master_od_1F80, &gateway_master_od_1F80sub0);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_2200);
		co_obj_insert_sub(&gateway_master_od_2200, &gateway_master_od_2200sub0);
		co_obj_insert_sub(&gateway_master_od_2200, &gateway_master_od_2200sub1);
		co_obj_insert_sub(&gateway_master_od_2200, &gateway_master_od_2200sub2);
		co_obj_insert_sub(&gateway_master_od_2200, &gateway_master_od_2200sub3);
		co_obj_insert_sub(&gateway_master_od_2200, &gateway_master_od_2200sub4);
		co_obj_insert_sub(&gateway_master_od_2200, &gateway_master_od_2200sub5);
		co_obj_insert_sub(&gateway_master_od_2200, &gateway_master_od_2200sub6);
		co_obj_insert_sub(&gateway_master_od_2200, &gateway_master_od_2200sub7);

		co_dev_insert_obj(&gateway_master_od, &gateway_master_od_2300);
		co_obj_insert_sub(&gateway_master_od_2300, &gateway_master_od_2300sub0);
		co_obj_insert_sub(&gateway_master_od_2300, &gateway_master_od_2300sub1);
		co_obj_insert_sub(&gateway_master_od_2300, &gateway_master_od_2300sub2);
		co_obj_insert_sub(&gateway_master_od_2300, &gateway_master_od_2300sub3);
		co_obj_insert_sub(&gateway_master_od_2300, &gateway_master_od_2300sub4);
		co_obj_insert_sub(&gateway_master_od_2300, &gateway_master_od_2300sub5);
		co_obj_insert_sub(&gateway_master_od_2300, &gateway_master_od_2300sub6);
		co_obj_insert_sub(&gateway_master_od_2300, &gateway_master_od_2300sub7);
	}
	return dev;
}
