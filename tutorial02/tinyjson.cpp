#include "tinyjson.h"
#include<assert.h>  // assert()
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, strtod() */

// 定义了一个用于检测json字符串的第一个字符是否为指定字符的宏
// 提前判断第一个字符是为了防止检测后续字符时指针越界
// 如检测 true 与 false 时，指针需要移动的次数就不同，不提前处理很有可能越界
#define EXPECT(c, ch)  do { assert(*c->json == (ch)); c->json++; } while(0)

// 定义用于检测字符是否为指定范围数字的宏
#define ISDIGIT(ch)  ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)  ((ch) >= '1' && (ch) <= '9')

/// @brief 为减少解析函数之间的参数传递，将数据放入此结构体中
typedef struct {
    const char* json;
} tiny_context;

/// @brief 解析 json 字符串中的空格
/// @param c tiny_context 
static void tiny_parse_whitespace(tiny_context* c){
    const char *p = c->json;
    // 当前字符为 json 语法中的空白时指针移动, json 语法详见：
    // https://tools.ietf.org/html/rfc7159 & https://tools.ietf.org/html/rfc5234
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'){
        p++;
    }
    c->json = p;
}

/// @brief 解析 json 字符串中的 null
/// @param c json 节点字符串
/// @param node json 节点
/// @return json 解析状态码
static int tiny_parse_null(tiny_context* c, tiny_node* node){
    EXPECT(c, 'n');  // 判断第一个值是否为 n
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l'){
        return TINY_PARSE_INVALID_VALUE;
    }
    c->json += 3;
    node->type = TINY_NULL;
    return TINY_PARSE_OK;  // 解析 null 完成
}

/// @brief 解析 json 字符串中的 true
/// @param c json 节点字符串
/// @param node json 节点
/// @return json 解析状态码
static int tiny_parse_true(tiny_context* c, tiny_node* node){
    EXPECT(c, 't');  // 判断第一个值是否为 t
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e'){
        return TINY_PARSE_INVALID_VALUE;
    }
    c->json += 3;
    node->type = TINY_TRUE;
    return TINY_PARSE_OK;  // 解析 null 完成
}

/// @brief 解析 json 字符串中的 false
/// @param c json 节点字符串
/// @param node json 节点
/// @return json 解析状态码
static int tiny_parse_false(tiny_context* c, tiny_node* node){
    EXPECT(c, 'f');  // 判断第一个值是否为 f
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e'){
        return TINY_PARSE_INVALID_VALUE;
    }
    c->json += 4;
    node->type = TINY_FALSE;
    return TINY_PARSE_OK;  // 解析 null 完成
}

// TODO: 功能更强大的数字解析

/// @brief 解析 json 字符串中的数字
/// @param c json 节点字符串
/// @param node json 节点
/// @return json 解析状态码
static int tiny_parse_number(tiny_context* c, tiny_node* node){
    const char* p = c->json;
    if (*p == '-') p++;
    if (*p == '0') p++;
    // 解析整数部分的数字
    else{
        if (!ISDIGIT1TO9(*p)) return TINY_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    // 解析小数部分的数字
    if (*p == '.'){
        p++;
        if (!ISDIGIT(*p)) return TINY_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    // 解析指数部分的数字
    if (*p == 'e' || *p == 'E'){
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return TINY_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;  // 程序异常的状态码
    node->n = strtod(c->json, nullptr);
    // 解析到过大的数字
    if (errno == ERANGE && (node->n == HUGE_VAL || node->n == -HUGE_VAL))
        return TINY_PARSE_NUMBER_TOO_BIG;
    node->type = TINY_NUMBER;
    c->json = p;
    return TINY_PARSE_OK;
}

/// @brief 分情况解析 json 字符串，
/// @param c json 节点字符串
/// @param node json节点
/// @return json 解析状态码
static int tiny_parse_value(tiny_context* c, tiny_node* node){
    switch (*c->json)
    {
    case 'n': return tiny_parse_null(c, node);
    case 't': return tiny_parse_true(c, node);
    case 'f': return tiny_parse_false(c, node);
    default:  return tiny_parse_number(c, node);
    case '\0': return TINY_PARSE_EXPECT_VALUE;  // '\0' 代表字符串结束符，因此可以判断是否为空
    }
}


int tiny_parse(tiny_node* node, const char* json){
    tiny_context c;
    int ret;
    assert(node != nullptr);
    c.json = json;  // 存储数据
    node->type = TINY_NULL;  // 先将节点类型置空
    tiny_parse_whitespace(&c);  // 处理空格
    if ((ret = tiny_parse_value(&c, node)) == TINY_PARSE_OK){
        tiny_parse_whitespace(&c);  // 处理字符串后空格
        if (*c.json != '\0'){  // 处理完后指向的不是字符串的结束符，即后面还有其他字符
            ret = TINY_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

tiny_type tiny_get_type(const tiny_node* node){
    assert(node != nullptr);  // 先判断是否为空指针
    return node->type;
}

double tiny_get_number(const tiny_node* node){
    assert(node != nullptr && tiny_get_type(node) == TINY_NUMBER);
    return node->n;
}