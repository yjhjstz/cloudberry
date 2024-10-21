#!/usr/bin/env python
import os
import sys
import random
import string


def generate_random_string(length):
    alphabet = string.ascii_letters + string.digits
    random_string = ''.join(random.choice(alphabet) for _ in range(length))
    return random_string

def gen_custom_complex_text_file(filename, row, delimiter, contentWide, isRandom):
    filename = "/home/gpadmin/%s" % (filename)
    f = open(filename, "wb")
    ran_str = ''.join(generate_random_string(contentWide))
    for i in range(0, row):
        len = contentWide
        if (isRandom):
            len = random.randint(1, contentWide)
        ss = "%s%s" %(ran_str[0 : len], delimiter)
        f.write(ss)
    f.close()

def gen_test_file():
    # multi delimiter used newline \r\n
    #gen text file range 100M ~ 2G
    print("gen test file crlf")
    gen_custom_complex_text_file("custom_file_crlf.txt", 100000000, "\r\n", 1, True)
    gen_custom_complex_text_file("custom_file_crlf2.txt", 11180000, "\r\n", 64, True)
    gen_custom_complex_text_file("custom_file_crlf3.txt", 2000, "\r\n", 1048576, True)
    gen_custom_complex_text_file("custom_file_crlf4.txt", 125, "\r\n", 33554432, True)

    # multi-delimiter and default delimter used newline \n
    #gen text file range 100M ~ 2G
    print("gen test file lf")
    gen_custom_complex_text_file("custom_file_lf.txt", 100000000, "\n", 1, True)
    gen_custom_complex_text_file("custom_file_lf2.txt", 11180000, "\n", 64, True)
    gen_custom_complex_text_file("custom_file_lf3.txt", 2000, "\n", 1048576, True)
    gen_custom_complex_text_file("custom_file_lf4.txt", 125, "\n", 33554432, True)

    # multi delimiter  used newline \r
    print("gen test file cr")
    gen_custom_complex_text_file("custom_file_cr.txt", 100000000, "\r", 1, True)
    gen_custom_complex_text_file("custom_file_cr2.txt", 11180000, "\r", 64, True)
    gen_custom_complex_text_file("custom_file_cr3.txt", 2000, "\r", 1048576, True)
    gen_custom_complex_text_file("custom_file_cr4.txt", 125, "\r", 33554432, True)


def gen_small_file():
    # multi delimiter used newline \r\n
    gen_custom_complex_text_file("custom_small_file_crlf.txt", 100, "\r\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_crlf2.txt", 100, "\r\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_crlf3.txt", 100, "\r\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_crlf4.txt", 100, "\r\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_crlf5.txt", 100, "\r\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_crlf6.txt", 100, "\r\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_crlf7.txt", 100, "\r\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_crlf8.txt", 100, "\r\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_crlf9.txt", 100, "\r\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_crlf10.txt", 100, "\r\n", 64, False)

    # multi-delimiter and default delimter used newline \n
    gen_custom_complex_text_file("custom_small_file_lf.txt", 100, "\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_lf2.txt", 100, "\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_lf3.txt", 100, "\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_lf4.txt", 100, "\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_lf5.txt", 100, "\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_lf6.txt", 100, "\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_lf7.txt", 100, "\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_lf8.txt", 100, "\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_lf9.txt", 100, "\n", 64, False)
    gen_custom_complex_text_file("custom_small_file_lf10.txt", 100, "\n", 64, False)

    # multi delimiter  used newline \r
    gen_custom_complex_text_file("custom_small_file_cr.txt", 100, "\r", 64, False)
    gen_custom_complex_text_file("custom_small_file_cr2.txt", 100, "\r", 64, False)
    gen_custom_complex_text_file("custom_small_file_cr3.txt", 100, "\r", 64, False)
    gen_custom_complex_text_file("custom_small_file_cr4.txt", 100, "\r", 64, False)
    gen_custom_complex_text_file("custom_small_file_cr5.txt", 100, "\r", 64, False)
    gen_custom_complex_text_file("custom_small_file_cr6.txt", 100, "\r", 64, False)
    gen_custom_complex_text_file("custom_small_file_cr7.txt", 100, "\r", 64, False)
    gen_custom_complex_text_file("custom_small_file_cr8.txt", 100, "\r", 64, False)
    gen_custom_complex_text_file("custom_small_file_cr9.txt", 100, "\r", 64, False)
    gen_custom_complex_text_file("custom_small_file_cr10.txt", 100, "\r", 64, False)



def add_to_hdfs():
    print("put test file to hdfs")
    os.system("hdfs dfs -rm -r /test/crlf/")
    os.system("hdfs dfs -rm -r /test/cr/")
    os.system("hdfs dfs -rm -r /test/lf/")

    os.system("hdfs dfs -mkdir /test/")
    os.system("hdfs dfs -mkdir /test/crlf/")
    os.system("hdfs dfs -mkdir /test/cr/")
    os.system("hdfs dfs -mkdir /test/lf/")

    # multi delimiter used newline \r\n
    os.system("hdfs dfs -put /home/gpadmin/custom_file_crlf.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_file_crlf2.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_file_crlf3.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_file_crlf4.txt /test/crlf/")

    # multi-delimiter and default delimter used newline \n
    os.system("hdfs dfs -put /home/gpadmin/custom_file_lf.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_file_lf2.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_file_lf3.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_file_lf4.txt /test/lf/")

    # multi delimiter  used newline \r
    os.system("hdfs dfs -put /home/gpadmin/custom_file_cr.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_file_cr2.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_file_cr3.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_file_cr4.txt /test/cr/")

    # add small file
    # multi delimiter used newline \r\n
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_crlf.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_crlf2.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_crlf3.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_crlf4.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_crlf5.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_crlf6.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_crlf7.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_crlf8.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_crlf9.txt /test/crlf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_crlf10.txt /test/crlf/")

    # multi-delimiter and default delimter used newline \n
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf2.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf3.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf4.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf5.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf6.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf7.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf8.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf9.txt /test/lf/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf10.txt /test/lf/")

    # multi delimiter  used newline \r
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_cr.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_cr2.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_cr3.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_cr4.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_cr5.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_cr6.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_cr7.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_cr8.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_cr9.txt /test/cr/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_cr10.txt /test/cr/")

def add_ignore_file():
    print("add ignore test file to hdfs")
    os.system("hdfs dfs -mkdir /ignore")
    os.system("hdfs dfs -mkdir /ignore/text")
    # os.system("hdfs dfs -mkdir /ignore/orc")
    # os.system("hdfs dfs -mkdir /ignore/avro")
    # os.system("hdfs dfs -mkdir /ignore/parquet")

    # text
    #case 1
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf.txt /ignore/text/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf.txt /ignore/text/.custom_small_file_lf.txt")

    #case 2
    os.system("hdfs dfs -mkdir /ignore/text/.ignore/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf.txt /ignore/text/.ignore/")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf.txt /ignore/text/.ignore/.custom_small_file_lf.txt")

    #case 3
    # os.system("hdfs dfs -mkdir /ignore/text/./")
    # os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf.txt /ignore/text/./")
    # os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf.txt /ignore/text/./.custom_small_file_lf.txt")

    #case 4
    os.system("hdfs dfs -mkdir /./")
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf.txt /./")

    #case 5
    os.system("hdfs dfs -put /home/gpadmin/custom_small_file_lf.txt /.custom_small_file_lf.txt")

gen_test_file()
gen_small_file()
add_to_hdfs()
add_ignore_file()