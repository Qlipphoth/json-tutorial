#include<iostream>
#include<cstdio>
#include<cstring>
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

// 用于测试返回值是否与期望的浮点型相同
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

// 用于测试返回的字符串是否与指定的相同
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")

// 测试返回值是否为指定的布尔型
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

// 测试节点值是否为期望的数字
#define TEST_NUMBER(expect, json)\
    do{\
        tiny_node node;\
        EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, json));\
        EXPECT_EQ_INT(TINY_NUMBER, tiny_get_type(&node));\
        EXPECT_EQ_DOUBLE(expect, tiny_get_number(&node));\
    } while(0)

// 测试错误情况的宏
#define TEST_ERROR(error, json)\
    do{\
        tiny_node node;\
        node.type = TINY_FALSE;\
        EXPECT_EQ_INT(error, tiny_parse(&node, json));\
        EXPECT_EQ_INT(TINY_NULL, tiny_get_type(&node));\
    } while(0)

// TODO: 重构 null、true、false的测试函数

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

/// @brief 测试数字解析
static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)\
    do {\
        tiny_node node;\
        tiny_init(&node);\
        EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, json));\
        EXPECT_EQ_INT(TINY_STRING, tiny_get_type(&node));\
        EXPECT_EQ_STRING(expect, tiny_get_string(&node), tiny_get_string_length(&node));\
        tiny_free(&node);\
    } while(0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
#if 1
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
#endif
}

/// @brief 测试解析数组
static void test_parse_array() {
    size_t i, j;
    tiny_node node;

    tiny_init(&node);
    EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, "[ ]"));
    EXPECT_EQ_INT(TINY_ARRAY, tiny_get_type(&node));
    EXPECT_EQ_SIZE_T(0, tiny_get_array_size(&node));
    tiny_free(&node);

    tiny_init(&node);
    EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(TINY_ARRAY, tiny_get_type(&node));
    EXPECT_EQ_SIZE_T(5, tiny_get_array_size(&node));
    EXPECT_EQ_INT(TINY_NULL,   tiny_get_type(tiny_get_array_element(&node, 0)));
    EXPECT_EQ_INT(TINY_FALSE,  tiny_get_type(tiny_get_array_element(&node, 1)));
    EXPECT_EQ_INT(TINY_TRUE,   tiny_get_type(tiny_get_array_element(&node, 2)));
    EXPECT_EQ_INT(TINY_NUMBER, tiny_get_type(tiny_get_array_element(&node, 3)));
    EXPECT_EQ_INT(TINY_STRING, tiny_get_type(tiny_get_array_element(&node, 4)));
    EXPECT_EQ_DOUBLE(123.0,  tiny_get_number(tiny_get_array_element(&node, 3)));
    EXPECT_EQ_STRING("abc",  tiny_get_string(tiny_get_array_element(&node, 4)), 
        tiny_get_string_length(tiny_get_array_element(&node, 4)));
    tiny_free(&node);

    tiny_init(&node);
    EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(TINY_ARRAY, tiny_get_type(&node));
    EXPECT_EQ_SIZE_T(4, tiny_get_array_size(&node));
    for (i = 0; i < 4; i++) {
        tiny_node *a = tiny_get_array_element(&node, i);
        EXPECT_EQ_INT(TINY_ARRAY, tiny_get_type(a));
        EXPECT_EQ_SIZE_T(i, tiny_get_array_size(a));
        for (j = 0; j < i; j++) {
            tiny_node *e = tiny_get_array_element(a, j);
            EXPECT_EQ_INT(TINY_NUMBER, tiny_get_type(e));
            EXPECT_EQ_DOUBLE((double)j, tiny_get_number(e));
        }
    }
    tiny_free(&node);
}

static void test_parse_object() {
    tiny_node node;
    size_t i;

    tiny_init(&node);
    EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, " { } "));
    EXPECT_EQ_INT(TINY_OBJECT, tiny_get_type(&node));
    EXPECT_EQ_SIZE_T(0, tiny_get_object_size(&node));
    tiny_free(&node);

    tiny_init(&node);
    EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node,
        " { "
        "\"n\" : null , "
        "\"f\" : false , "
        "\"t\" : true , "
        "\"i\" : 123 , "
        "\"s\" : \"abc\", "
        "\"a\" : [ 1, 2, 3 ],"
        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
        " } "
    ));
    EXPECT_EQ_INT(TINY_OBJECT, tiny_get_type(&node));
    EXPECT_EQ_SIZE_T(7, tiny_get_object_size(&node));
    EXPECT_EQ_STRING("n", tiny_get_object_key(&node, 0), tiny_get_object_key_length(&node, 0));
    EXPECT_EQ_INT(TINY_NULL,   tiny_get_type(tiny_get_object_value(&node, 0)));
    EXPECT_EQ_STRING("f", tiny_get_object_key(&node, 1), tiny_get_object_key_length(&node, 1));
    EXPECT_EQ_INT(TINY_FALSE,  tiny_get_type(tiny_get_object_value(&node, 1)));
    EXPECT_EQ_STRING("t", tiny_get_object_key(&node, 2), tiny_get_object_key_length(&node, 2));
    EXPECT_EQ_INT(TINY_TRUE,   tiny_get_type(tiny_get_object_value(&node, 2)));
    EXPECT_EQ_STRING("i", tiny_get_object_key(&node, 3), tiny_get_object_key_length(&node, 3));
    EXPECT_EQ_INT(TINY_NUMBER, tiny_get_type(tiny_get_object_value(&node, 3)));
    EXPECT_EQ_DOUBLE(123.0, tiny_get_number(tiny_get_object_value(&node, 3)));
    EXPECT_EQ_STRING("s", tiny_get_object_key(&node, 4), tiny_get_object_key_length(&node, 4));
    EXPECT_EQ_INT(TINY_STRING, tiny_get_type(tiny_get_object_value(&node, 4)));
    EXPECT_EQ_STRING("abc", tiny_get_string(tiny_get_object_value(&node, 4)), tiny_get_string_length(tiny_get_object_value(&node, 4)));
    EXPECT_EQ_STRING("a", tiny_get_object_key(&node, 5), tiny_get_object_key_length(&node, 5));
    EXPECT_EQ_INT(TINY_ARRAY, tiny_get_type(tiny_get_object_value(&node, 5)));
    EXPECT_EQ_SIZE_T(3, tiny_get_array_size(tiny_get_object_value(&node, 5)));
    for (i = 0; i < 3; i++) {
        tiny_node* e = tiny_get_array_element(tiny_get_object_value(&node, 5), i);
        EXPECT_EQ_INT(TINY_NUMBER, tiny_get_type(e));
        EXPECT_EQ_DOUBLE(i + 1.0, tiny_get_number(e));
    }
    EXPECT_EQ_STRING("o", tiny_get_object_key(&node, 6), tiny_get_object_key_length(&node, 6));
    {
        tiny_node* o = tiny_get_object_value(&node, 6);
        EXPECT_EQ_INT(TINY_OBJECT, tiny_get_type(o));
        for (i = 0; i < 3; i++) {
            tiny_node* ov = tiny_get_object_value(o, i);
            EXPECT_TRUE('1' + i == tiny_get_object_key(o, i)[0]);
            EXPECT_EQ_SIZE_T(1, tiny_get_object_key_length(o, i));
            EXPECT_EQ_INT(TINY_NUMBER, tiny_get_type(ov));
            EXPECT_EQ_DOUBLE(i + 1.0, tiny_get_number(ov));
        }
    }
    tiny_free(&node);
}


/// @brief 测试输入为空白的情况
static void test_parse_expect_value(){
    TEST_ERROR(TINY_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(TINY_PARSE_EXPECT_VALUE, " ");
}

/// @brief 测试输入为非法字符的情况
static void test_parse_invalid_value(){
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "?");

#if 1
    /* invalid number */
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "nan");
#endif

#if 1
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "[1,]");
    TEST_ERROR(TINY_PARSE_INVALID_VALUE, "[\"a\", nul]");  
#endif

}

/// @brief 测试一个值后除了空白还有其他值的情况
static void test_parse_root_not_singular(){
    TEST_ERROR(TINY_PARSE_ROOT_NOT_SINGULAR, "null x");

#if 1
    /* invalid number */
    TEST_ERROR(TINY_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_ERROR(TINY_PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(TINY_PARSE_ROOT_NOT_SINGULAR, "0x123");
    TEST_ERROR(TINY_PARSE_ROOT_NOT_SINGULAR, "0.1.1.1");
#endif

}

/// @brief 测试数字越界的情况
static void test_parse_number_too_big() {
#if 1
    TEST_ERROR(TINY_PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(TINY_PARSE_NUMBER_TOO_BIG, "-1e309");
#endif
}

static void test_parse_missing_quotation_mark() {
    TEST_ERROR(TINY_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(TINY_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
#if 1
    TEST_ERROR(TINY_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(TINY_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(TINY_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(TINY_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
#endif
}

static void test_parse_invalid_string_char() {
#if 1
    TEST_ERROR(TINY_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(TINY_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
#endif
}

static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
}

static void test_parse_invalid_unicode_surrogate() {
#if 1
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(TINY_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
#endif
}

static void test_parse_miss_comma_or_square_bracket() {
#if 1
    TEST_ERROR(TINY_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(TINY_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(TINY_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(TINY_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
#endif
}

static void test_parse_miss_key() {
    TEST_ERROR(TINY_PARSE_MISS_KEY, "{:1,");
    TEST_ERROR(TINY_PARSE_MISS_KEY, "{1:1,");
    TEST_ERROR(TINY_PARSE_MISS_KEY, "{true:1,");
    TEST_ERROR(TINY_PARSE_MISS_KEY, "{false:1,");
    TEST_ERROR(TINY_PARSE_MISS_KEY, "{null:1,");
    TEST_ERROR(TINY_PARSE_MISS_KEY, "{[]:1,");
    TEST_ERROR(TINY_PARSE_MISS_KEY, "{{}:1,");
    TEST_ERROR(TINY_PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
    TEST_ERROR(TINY_PARSE_MISS_COLON, "{\"a\"}");
    TEST_ERROR(TINY_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
    TEST_ERROR(TINY_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(TINY_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(TINY_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(TINY_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

/// @brief 进行所有测试
static void test_parse(){
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();
    test_parse_array();
    test_parse_object();

    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    test_parse_miss_comma_or_square_bracket();

#if 1
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
#endif
}

#define TEST_ROUNDTRIP(json)\
    do {\
        tiny_node node;\
        char* json2;\
        size_t length;\
        tiny_init(&node);\
        EXPECT_EQ_INT(TINY_PARSE_OK, tiny_parse(&node, json));\
        json2 = tiny_stringify(&node, &length);\
        EXPECT_EQ_STRING(json, json2, length);\
        tiny_free(&node);\
        free(json2);\
    } while(0)

static void test_stringify_number() {
    TEST_ROUNDTRIP("0");
    TEST_ROUNDTRIP("-0");
    TEST_ROUNDTRIP("1");
    TEST_ROUNDTRIP("-1");
    TEST_ROUNDTRIP("1.5");
    TEST_ROUNDTRIP("-1.5");
    TEST_ROUNDTRIP("3.25");
    TEST_ROUNDTRIP("1e+20");
    TEST_ROUNDTRIP("1.234e+20");
    TEST_ROUNDTRIP("1.234e-20");

    TEST_ROUNDTRIP("1.0000000000000002"); /* the smallest number > 1 */
    TEST_ROUNDTRIP("4.9406564584124654e-324"); /* minimum denormal */
    TEST_ROUNDTRIP("-4.9406564584124654e-324");
    TEST_ROUNDTRIP("2.2250738585072009e-308");  /* Max subnormal double */
    TEST_ROUNDTRIP("-2.2250738585072009e-308");
    TEST_ROUNDTRIP("2.2250738585072014e-308");  /* Min normal positive double */
    TEST_ROUNDTRIP("-2.2250738585072014e-308");
    TEST_ROUNDTRIP("1.7976931348623157e+308");  /* Max double */
    TEST_ROUNDTRIP("-1.7976931348623157e+308");
}

static void test_stringify_string() {
    TEST_ROUNDTRIP("\"\"");
    TEST_ROUNDTRIP("\"Hello\"");
    TEST_ROUNDTRIP("\"Hello\\nWorld\"");
    TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
    TEST_ROUNDTRIP("\"Hello\\u0000World\"");
}

static void test_stringify_array() {
    TEST_ROUNDTRIP("[]");
    TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object() {
    TEST_ROUNDTRIP("{}");
    TEST_ROUNDTRIP("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
}

static void test_stringify() {
    TEST_ROUNDTRIP("null");
    TEST_ROUNDTRIP("false");
    TEST_ROUNDTRIP("true");
    test_stringify_number();
    test_stringify_string();
    test_stringify_array();
    test_stringify_object();
}

static void test_access_null() {
    tiny_node node;
    tiny_init(&node);
    // 先把值设为字符串，那么做可以测试设置其他类型时，有没有调用 `tiny_free()` 去释放内存。
    tiny_set_string(&node, "a", 1);
    tiny_set_null(&node);
    EXPECT_EQ_INT(TINY_NULL, tiny_get_type(&node));
    tiny_free(&node);
}

static void test_access_boolean() {
    tiny_node node;
    tiny_init(&node);
    tiny_set_string(&node, "a", 1);
    tiny_set_boolean(&node, 1);
    EXPECT_TRUE(tiny_get_boolean(&node));
    tiny_set_boolean(&node, 0);
    EXPECT_FALSE(tiny_get_boolean(&node));
    tiny_free(&node);
}

static void test_access_number() {
    tiny_node node;
    tiny_init(&node);
    tiny_set_string(&node, "a", 1);
    tiny_set_number(&node, 1234.5);
    EXPECT_EQ_DOUBLE(1234.5, tiny_get_number(&node));
    tiny_free(&node);
}

static void test_access_string() {
    tiny_node node;
    tiny_init(&node);
    tiny_set_string(&node, "", 0);
    EXPECT_EQ_STRING("", tiny_get_string(&node), tiny_get_string_length(&node));
    tiny_set_string(&node, "Hello", 5);
    EXPECT_EQ_STRING("Hello", tiny_get_string(&node), tiny_get_string_length(&node));
    tiny_free(&node);
}

static void test_access_array() {
    tiny_node a, e;
    size_t i, j;

    tiny_init(&a);

    for (j = 0; j <= 5; j += 5) {
        tiny_set_array(&a, j);
        EXPECT_EQ_SIZE_T(0, tiny_get_array_size(&a));
        EXPECT_EQ_SIZE_T(j, tiny_get_array_capacity(&a));
        for (i = 0; i < 10; i++) {
            tiny_init(&e);
            tiny_set_number(&e, i);
            tiny_move(tiny_pushback_array_element(&a), &e);
            tiny_free(&e);
        }

        EXPECT_EQ_SIZE_T(10, tiny_get_array_size(&a));
        for (i = 0; i < 10; i++)
            EXPECT_EQ_DOUBLE((double)i, tiny_get_number(tiny_get_array_element(&a, i)));
    }

    tiny_popback_array_element(&a);
    EXPECT_EQ_SIZE_T(9, tiny_get_array_size(&a));
    for (i = 0; i < 9; i++)
        EXPECT_EQ_DOUBLE((double)i, tiny_get_number(tiny_get_array_element(&a, i)));

    tiny_erase_array_element(&a, 4, 0);
    EXPECT_EQ_SIZE_T(9, tiny_get_array_size(&a));
    for (i = 0; i < 9; i++)
        EXPECT_EQ_DOUBLE((double)i, tiny_get_number(tiny_get_array_element(&a, i)));

    tiny_erase_array_element(&a, 8, 1);
    EXPECT_EQ_SIZE_T(8, tiny_get_array_size(&a));
    for (i = 0; i < 8; i++)
        EXPECT_EQ_DOUBLE((double)i, tiny_get_number(tiny_get_array_element(&a, i)));

    tiny_erase_array_element(&a, 0, 2);
    EXPECT_EQ_SIZE_T(6, tiny_get_array_size(&a));
    for (i = 0; i < 6; i++)
        EXPECT_EQ_DOUBLE((double)i + 2, tiny_get_number(tiny_get_array_element(&a, i)));

    for (i = 0; i < 2; i++) {
        tiny_init(&e);
        tiny_set_number(&e, i);
        tiny_move(tiny_insert_array_element(&a, i), &e);
        tiny_free(&e);
    }
    
    EXPECT_EQ_SIZE_T(8, tiny_get_array_size(&a));
    for (i = 0; i < 8; i++)
        EXPECT_EQ_DOUBLE((double)i, tiny_get_number(tiny_get_array_element(&a, i)));

    EXPECT_TRUE(tiny_get_array_capacity(&a) > 8);
    tiny_shrink_array(&a);
    EXPECT_EQ_SIZE_T(8, tiny_get_array_capacity(&a));
    EXPECT_EQ_SIZE_T(8, tiny_get_array_size(&a));
    for (i = 0; i < 8; i++)
        EXPECT_EQ_DOUBLE((double)i, tiny_get_number(tiny_get_array_element(&a, i)));

    tiny_set_string(&e, "Hello", 5);
    tiny_move(tiny_pushback_array_element(&a), &e);     /* Test if element is freed */
    tiny_free(&e);

    i = tiny_get_array_capacity(&a);
    tiny_clear_array(&a);
    EXPECT_EQ_SIZE_T(0, tiny_get_array_size(&a));
    EXPECT_EQ_SIZE_T(i, tiny_get_array_capacity(&a));   /* capacity remains unchanged */
    tiny_shrink_array(&a);
    EXPECT_EQ_SIZE_T(0, tiny_get_array_capacity(&a));

    tiny_free(&a);
}

static void test_access_object() {
#if 1
    tiny_node o, v, *pv;
    size_t i, j, index;

    tiny_init(&o);

    for (j = 0; j <= 5; j += 5) {
        tiny_set_object(&o, j);
        EXPECT_EQ_SIZE_T(0, tiny_get_object_size(&o));
        EXPECT_EQ_SIZE_T(j, tiny_get_object_capacity(&o));
        for (i = 0; i < 10; i++) {
            char key[2] = "a";
            key[0] += i;  // a b c d ...
            tiny_init(&v);
            tiny_set_number(&v, i);
            tiny_move(tiny_set_object_key(&o, key, 1), &v);  // {'a' : 1 , 'b' : 2 , ...}
            tiny_free(&v);
        }
        EXPECT_EQ_SIZE_T(10, tiny_get_object_size(&o));
        for (i = 0; i < 10; i++) {
            char key[] = "a";
            key[0] += i;
            index = tiny_find_object_index(&o, key, 1);
            EXPECT_TRUE(index != TINY_KEY_NOT_EXIST);
            pv = tiny_get_object_value(&o, index);
            EXPECT_EQ_DOUBLE((double)i, tiny_get_number(pv));
        }
    }

    index = tiny_find_object_index(&o, "j", 1);    
    EXPECT_TRUE(index != TINY_KEY_NOT_EXIST);
    tiny_remove_object(&o, index);
    index = tiny_find_object_index(&o, "j", 1);
    EXPECT_TRUE(index == TINY_KEY_NOT_EXIST);
    EXPECT_EQ_SIZE_T(9, tiny_get_object_size(&o));

    index = tiny_find_object_index(&o, "a", 1);
    EXPECT_TRUE(index != TINY_KEY_NOT_EXIST);
    tiny_remove_object(&o, index);
    index = tiny_find_object_index(&o, "a", 1);
    EXPECT_TRUE(index == TINY_KEY_NOT_EXIST);
    EXPECT_EQ_SIZE_T(8, tiny_get_object_size(&o));

    EXPECT_TRUE(tiny_get_object_capacity(&o) > 8);
    tiny_shrink_object(&o);
    EXPECT_EQ_SIZE_T(8, tiny_get_object_capacity(&o));
    EXPECT_EQ_SIZE_T(8, tiny_get_object_size(&o));
    for (i = 0; i < 8; i++) {
        char key[] = "a";
        key[0] += i + 1;
        EXPECT_EQ_DOUBLE((double)i + 1, tiny_get_number(tiny_get_object_value(&o, tiny_find_object_index(&o, key, 1))));
    }

    tiny_set_string(&v, "Hello", 5);
    tiny_move(tiny_set_object_key(&o, "World", 5), &v); /* Test if element is freed */
    tiny_free(&v);

    pv = tiny_find_object_value(&o, "World", 5);
    EXPECT_TRUE(pv != nullptr);
    EXPECT_EQ_STRING("Hello", tiny_get_string(pv), tiny_get_string_length(pv));

    i = tiny_get_object_capacity(&o);
    tiny_clear_object(&o);
    EXPECT_EQ_SIZE_T(0, tiny_get_object_size(&o));
    EXPECT_EQ_SIZE_T(i, tiny_get_object_capacity(&o)); /* capacity remains unchanged */
    tiny_shrink_object(&o);
    EXPECT_EQ_SIZE_T(0, tiny_get_object_capacity(&o));

    tiny_free(&o);
#endif
}

static void test_access() {
    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
    test_access_array();
    test_access_object();
}

int main(){
    test_parse();
    test_stringify();
    test_access();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}


