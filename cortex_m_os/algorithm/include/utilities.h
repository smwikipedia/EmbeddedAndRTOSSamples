#ifndef ALGO_UTILITIES_H
#define ALGO_UTILITIES_H

#define NULL_PTR ((void*)0)

#define OFFSET_OF(field_name, struct_name) ((uint32_t)&(((struct_name*)(0))->field_name))

/*
The containing struct of the field.
*/
#define CS(field_ptr, field_name, struct_name) ((struct_name*)(((uint32_t)field_ptr) - OFFSET_OF (field_name, struct_name)))

#endif
