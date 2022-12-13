/*
 * elf-common.h
 *
 * Copyright (C) 2014 Seth Jennings <sjenning@redhat.com>
 * Copyright (C) 2013-2014 Josh Poimboeuf <jpoimboe@redhat.com>
 * Copyright (C) 2022 Longjun Luo <luolongjun@huawei.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA,
 * 02110-1301, USA.
 */

#ifndef __UPATCH_ELF_COMMON_H_
#define __UPATCH_ELF_COMMON_H_

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <gelf.h>

#include "upatch-elf.h"
#include "list.h"
#include "log.h"

#define ALLOC_LINK(_new, _list) \
{ \
	(_new) = calloc(1, sizeof(*(_new))); \
	if (!(_new)) \
		ERROR("calloc"); \
	INIT_LIST_HEAD(&(_new)->list); \
	if (_list) \
		list_add_tail(&(_new)->list, (_list)); \
}

static inline bool is_rela_section(struct section *sec)
{
    /*
     * An architecture usually only accepts one type.
     * And, X86_64 only uses RELA
     */
	return (sec->sh.sh_type == SHT_RELA);
}


static inline bool is_text_section(struct section *sec)
{
	return (sec->sh.sh_type == SHT_PROGBITS &&
		(sec->sh.sh_flags & SHF_EXECINSTR));
}

static inline bool is_string_section(struct section *sec)
{
	return !strncmp(sec->name, ".rodata", 7);
}

static inline bool is_debug_section(struct section *sec)
{
	char *name;
	if (is_rela_section(sec))
		name = sec->base->name;
	else
		name = sec->name;

	return !strncmp(name, ".debug_", 7);
}

static inline bool is_eh_frame_section(struct section *sec)
{
	char *name;
	if (is_rela_section(sec))
		name = sec->base->name;
	else
		name = sec->name;

	return !strncmp(name, ".eh_frame", 9);
}

static inline struct symbol *find_symbol_by_index(struct list_head *list, size_t index)
{
	struct symbol *sym;

	list_for_each_entry(sym, list, list)
		if (sym->index == index)
			return sym;

	return NULL;
}

static inline struct symbol *find_symbol_by_name(struct list_head *list, const char *name)
{
	struct symbol *sym;

	list_for_each_entry(sym, list, list)
		if (sym->name && !strcmp(sym->name, name))
			return sym;

	return NULL;
}

static inline struct section *find_section_by_index(struct list_head *list, unsigned int index)
{
	struct section *sec;

	list_for_each_entry(sec, list, list)
		if (sec->index == index)
			return sec;

	return NULL;
}

static inline struct section *find_section_by_name(struct list_head *list, const char *name)
{
	struct section *sec;

	list_for_each_entry(sec, list, list)
		if (!strcmp(sec->name, name))
			return sec;

	return NULL;
}

// section like .rodata.str1.
static inline bool is_string_literal_section(struct section *sec)
{
	return !strncmp(sec->name, ".rodata.", 8) && strstr(sec->name, ".str");
}

static bool has_digit_tail(char *tail)
{
	if (*tail != '.')
		return false;

	while (isdigit(*++tail))
		;

	if (!*tail)
		return true;

	return false;
}

/*
 * Compare gcc-mangled symbols. It skips the comparision of any substring
 * which consists of '.' followed by any number of digits.
 * TODO: This function is not necessary for userpace, more examples are needed.
 */
int mangled_strcmp(char *, char *);


/*
 * TODO: Special static local variables should never be correlated and should always
 * be included if they are referenced by an included function.
 */
static inline bool is_special_static(struct symbol *sym){
    /* Not need it now. */
    return false;
}

bool is_normal_static_local(struct symbol *);

static inline char *section_function_name(struct section *sec)
{
	if (is_rela_section(sec))
		sec = sec->base;
	return sec->sym ? sec->sym->name : sec->name;
}

static inline char *status_str(enum status status)
{
	switch(status) {
	case NEW:
		return "NEW";
	case CHANGED:
		return "CHANGED";
	case SAME:
		return "SAME";
	default:
		ERROR("status_str");
	}
	return NULL;
}

int offset_of_string(struct list_head *, char *);

static inline unsigned int absolute_rela_type(struct upatch_elf *uelf)
{
	switch(uelf->arch) {
	case AARCH64:
		return R_AARCH64_ABS64;
	case X86_64:
		return R_X86_64_64;
	default:
		ERROR("unsupported arch");
	}
	return 0;
}

static inline bool is_null_sym(struct symbol *sym)
{
	return !strlen(sym->name);
}

static inline bool is_file_sym(struct symbol *sym)
{
	return sym->type == STT_FILE;
}

static inline bool is_local_func_sym(struct symbol *sym)
{
	return sym->bind == STB_LOCAL && sym->type == STT_FUNC;
}

static inline bool is_local_sym(struct symbol *sym)
{
	return sym->bind == STB_LOCAL;
}

bool is_gcc6_localentry_bundled_sym(struct upatch_elf *, struct symbol *);

/*
 * Mapping symbols are used to mark and label the transitions between code and
 * data in elf files. They begin with a "$" dollar symbol. Don't correlate them
 * as they often all have the same name either "$x" to mark the start of code
 * or "$d" to mark the start of data.
 */
bool is_mapping_symbol(struct upatch_elf *, struct symbol *);

#endif