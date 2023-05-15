#include<iostream>
#include<cstdio>
#include "tinyjson.h"
using namespace std;

static int main_ret = 0;  // main 函数返回值
static int test_count = 0;  // 测试案例个数
static int test_pass = 0;  // 通过的个数

// 定义了一个用于测试的宏
// equality: 评判标准
// expect: 期望值
// actual: 实际值
// format: 数据打印格式
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

// 用于测试返回值是否与期望的整型相同的情况
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")


/// @brief 测试是否能解析出 null 
static void test_parse_null(){
    tiny_node node;  // 一个 json 节点
    node.type = TINY_FALSE;  // 先将节点数据类型置为 false
    EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, "null"));  // 应正常解析，返回 OK 状态码
    EXPECT_EQ_INT(TINY_NULL, tiny_get_type(&node));  // 若正确解析完，节点数据类型应被置为 null 
}

/// @brief 测试是否能解析出 true 
static void test_parse_true(){
    tiny_node node;  // 一个 json 节点
    node.type = TINY_FALSE;  // 先将节点数据类型置为 false
    EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, "true"));  // 应正常解析，返回 OK 状态码
    EXPECT_EQ_INT(TINY_TRUE, tiny_get_type(&node));  // 若正确解析完，节点数据类型应被置为 null 
}

/// @brief 测试是否能解析出 false
static void test_parse_false(){
    tiny_node node;  // 一个 json 节点
    node.type = TINY_FALSE;  // 先将节点数据类型置为 false
    EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, "false"));  // 应正常解析，返回 OK 状态码
    EXPECT_EQ_INT(TINY_FALSE, tiny_get_type(&node));  // 若正确解析完，节点数据类型应被置为 null 
}

/// @brief 测试输入为空白的情况
static void test_parse_expect_value(){
    tiny_node node;

    node.type = TINY_FALSE;
    EXPECT_EQ_INT(TINY_PARSE_EXPECT_VALUE, tiny_parse(&node, ""));  // 无输入
    EXPECT_EQ_INT(TINY_NULL, tiny_get_type(&node));

    node.type = TINY_FALSE;
    EXPECT_EQ_INT(TINY_PARSE_EXPECT_VALUE, tiny_parse(&node, " "));  // 仅空格也算无输入
    EXPECT_EQ_INT(TINY_NULL, tiny_get_type(&node));
}

/// @brief 测试输入为非法字符的情况
static void test_parse_invalid_value(){
    tiny_node node;
    node.type = TINY_FALSE;
    EXPECT_EQ_INT(TINY_PARSE_INVALID_VALUE, tiny_parse(&node, "nul"));
    EXPECT_EQ_INT(TINY_NULL, tiny_get_type(&node));

    node.type = TINY_FALSE;
    EXPECT_EQ_INT(TINY_PARSE_INVALID_VALUE, tiny_parse(&node, "?"));
    EXPECT_EQ_INT(TINY_NULL, tiny_get_type(&node));
}

/// @brief 测试一个值后除了空白还有其他值的情况
static void test_parse_root_not_singular(){
    tiny_node node;
    node.type = TINY_FALSE;
    EXPECT_EQ_INT(TINY_PARSE_ROOT_NOT_SINGULAR, tiny_parse(&node, "null x"));
    EXPECT_EQ_INT(TINY_NULL, tiny_get_type(&node));
}

/// @brief 进行所有测试
static void test_parse(){
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main(){
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}


