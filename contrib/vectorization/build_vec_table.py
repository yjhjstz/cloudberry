import re

def parse_generic_line(line):
    pattern = r"(\w+)\s*=>\s*'([^']*)'"
    matches = re.findall(pattern, line)

    result = {}
    for match in matches:
        key, value = match
        result[key] = value.strip('\'')
    return result

def parse_file(filename):
    with open(filename, 'r') as file:
        data = file.read().strip()
        entries = data.split("},")
        results = []

        for entry in entries:
            result = parse_generic_line(entry.strip())
            if result:
                results.append(result)
    return results


filename="src/include/utils/arrow_fmgr.h"
file =  open(filename, 'w')
file.write("""
/*-------------------------------------------------------------------------
 *
 * arrow_fmgr.c
 * *
 * Copyright (c) 2016-Present Hashdata, Inc.
 *
 *
 * IDENTIFICATION
 *	  src/backend/utils/arrow/arrow_fmgr.h
 *
 *-------------------------------------------------------------------------
 */
""")

support_proc=parse_file("pg_vec.dat")
file.write("#define ARROW_FMGR ")
for i in range(len(support_proc)):
	oid=support_proc[i]
	file.write(f"{{{oid['procoid']}, {oid['descr']}, {oid['funcName']}, {oid['buildfun']}, {oid['checksupportfun']}}}")
	if i != len(support_proc) - 1:
		file.write(f", \\\n")
file.write("\n")
file.close()