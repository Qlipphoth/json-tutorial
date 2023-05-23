#ifndef TINYJSON_H__
#define TINYJSON_H__

#include<stddef.h>  /* size_t */
// size_t 类型表示C中任何对象所能达到的最大长度，它是无符号整数。
// 在声明诸如字符数或者数组索引这样的长度变量时用size_t 是好的做法。

// 定义 json 中的 7 种数据类型（bool 中的 true 和 false 分开）
typedef enum { TINY_NULL, TINY_FALSE, TINY_TRUE, TINY_NUMBER, TINY_STRING, 
    TINY_ARRAY, TINY_OBJECT } tiny_type;

// 定义 json 中的节点结构 tiny_node，节点中包含 tiny_type 类型的值
// 教程中名称是 _value，但我总觉得变扭，改用 node
typedef struct tiny_node tiny_node;  // 前向声明（forward declare）tiny_node 类型

struct tiny_node {
    // union 用于使多个变量共用一块内存，节省空间
    union 
    {
        /* 数组中元素的数据类型也为一个json节点，可继续存放数、数字、字符串 */
        struct {tiny_node* e; size_t size;}a;  /* array:  elements, element count */
        struct { char* s; size_t len; }s;         /* string: null-terminated string, string length */
        double n;                                 /* number */
    };
    tiny_type type;
};

/// @brief 枚举类型，代表解析状态码
enum {
    // 赋值了第一个变量为 0，后面的值会依次递增
    TINY_PARSE_OK = 0,  // 正确解析
    TINY_PARSE_EXPECT_VALUE,  // 节点值为空白 
    TINY_PARSE_INVALID_VALUE,  // 节点中有非法的值 
    TINY_PARSE_ROOT_NOT_SINGULAR,  // 一个值之后除了空白还有其他字符 
    TINY_PARSE_NUMBER_TOO_BIG,  // 解析出的数字过大 
    TINY_PARSE_MISS_QUOTATION_MARK,  // 缺少字符串结尾的 \"
    TINY_PARSE_INVALID_STRING_ESCAPE,  // 无效的字符串转义
    TINY_PARSE_INVALID_STRING_CHAR,  // 无效字符
    TINY_PARSE_INVALID_UNICODE_HEX,  // 非法的十六进制数
    TINY_PARSE_INVALID_UNICODE_SURROGATE,  // 非法的代理对范围
    TINY_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,  // 缺少逗号或方括号
};

// 提供 json 节点的初始化宏
#define tiny_init(node) do { (node)->type = TINY_NULL; } while(0)

/// @brief 解析 json 节点中的值并返回int类型的解析状态码
/// @param node json 节点
/// @param json 该节点中的字符串类型的值
/// @return 解析状态码
int tiny_parse(tiny_node* node, const char* json);

/// @brief 释放节点内存
/// @param node json 节点
void tiny_free(tiny_node* node);

/// @brief 返回解析出的节点中的数据类型
/// @param node json 节点
/// @return json 数据类型
tiny_type tiny_get_type(const tiny_node* node);

#define tiny_set_null(node) tiny_free(node)

int tiny_get_boolean(const tiny_node* node);
void tiny_set_boolean(tiny_node* node, int b);

double tiny_get_number(const tiny_node* node);
void tiny_set_number(tiny_node* node, double n);

const char* tiny_get_string(const tiny_node* node);
size_t tiny_get_string_length(const tiny_node* node);
void tiny_set_string(tiny_node* node, const char* s, size_t len);

size_t tiny_get_array_size(const tiny_node* node);
tiny_node* tiny_get_array_element(const tiny_node* node, size_t index);

#endif
