#ifndef TINYJSON_H__
#define TINYJSON_H__

#include<stddef.h>  /* size_t */
// size_t 类型表示C中任何对象所能达到的最大长度，它是无符号整数。
// 在声明诸如字符数或者数组索引这样的长度变量时用size_t 是好的做法。

// 定义 json 中的 7 种数据类型（bool 中的 true 和 false 分开）
typedef enum { TINY_NULL, TINY_FALSE, TINY_TRUE, TINY_NUMBER, TINY_STRING, 
    TINY_ARRAY, TINY_OBJECT } tiny_type;

#define TINY_KEY_NOT_EXIST ((size_t) - 1)

// 定义 json 中的节点结构 tiny_node，节点中包含 tiny_type 类型的值
// 教程中名称是 _value，但我总觉得变扭，改用 node
typedef struct tiny_node tiny_node;  // 前向声明（forward declare）tiny_node 类型
typedef struct tiny_member tiny_member;

struct tiny_node {
    // union 用于使多个变量共用一块内存，节省空间
    union 
    {
        struct { tiny_member* m; size_t size, capacity; }o;
        /* 数组中元素的数据类型也为一个json节点，可继续存放数、数字、字符串 */
        struct { tiny_node* e; size_t size, capacity; }a;  /* array:  elements, element count */
        struct { char* s; size_t len; }s;         /* string: null-terminated string, string length */
        double n;                                 /* number */
    };
    tiny_type type;
};

/// @brief 存放 json 对象类型的数据结构, 由 key : value 组成
struct tiny_member
{
    char* key; size_t keylen; /* member key string, key string length */
    tiny_node value;           /* member value */
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
    TINY_PARSE_INVALID_UNICODE_HEX,  // 无效的十六进制数
    TINY_PARSE_INVALID_UNICODE_SURROGATE,  // 无效的代理对范围
    TINY_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,  // 缺少逗号或方括号
    TINY_PARSE_MISS_KEY,  // 缺少 key
    TINY_PARSE_MISS_COLON,  // 缺少 : 
    TINY_PARSE_MISS_COMMA_OR_CURLY_BRACKET,  // 缺少 , or }
};

// 提供 json 节点的初始化宏
#define tiny_init(node) do { (node)->type = TINY_NULL; } while(0)

int tiny_parse(tiny_node* node, const char* json);
char* tiny_stringify(const tiny_node* node, size_t* length);

void tiny_copy(tiny_node* node, const tiny_node* src);
void tiny_move(tiny_node* dst, tiny_node* src);
void tiny_swap(tiny_node& lhs, tiny_node* rhs);

void tiny_free(tiny_node* node);

tiny_type tiny_get_type(const tiny_node* node);
int tiny_is_equal(const tiny_node* lhs, const tiny_node* rhs);

#define tiny_set_null(node) tiny_free(node)

int tiny_get_boolean(const tiny_node* node);
void tiny_set_boolean(tiny_node* node, int b);

double tiny_get_number(const tiny_node* node);
void tiny_set_number(tiny_node* node, double n);

const char* tiny_get_string(const tiny_node* node);
size_t tiny_get_string_length(const tiny_node* node);
void tiny_set_string(tiny_node* node, const char* s, size_t len);

void tiny_set_array(tiny_node* node, size_t capacity);
size_t tiny_get_array_size(const tiny_node* node);
size_t tiny_get_array_capacity(const tiny_node* node);
void tiny_reserve_array(tiny_node* node, size_t capacity);
void tiny_shrink_array(tiny_node* node);
void tiny_clear_array(tiny_node* node);
tiny_node* tiny_get_array_element(const tiny_node* node, size_t index);
tiny_node* tiny_pushback_array_element(tiny_node* node);
void tiny_popback_array_element(tiny_node* node);
tiny_node* tiny_insert_array_element(tiny_node* node, size_t index);
void tiny_erase_array_element(tiny_node* node, size_t index, size_t count);

void tiny_set_object(tiny_node* node, size_t capacity);
size_t tiny_get_object_size(const tiny_node* node);
size_t tiny_get_object_capacity(const tiny_node* node);
void tiny_reserve_object(tiny_node* node, size_t capacity);
void tiny_shrink_object(tiny_node* node);
void tiny_clear_object(tiny_node* node);
const char* tiny_get_object_key(const tiny_node* node, size_t index);
size_t tiny_get_object_key_length(const tiny_node* node, size_t index);
tiny_node* tiny_get_object_value(const tiny_node* node, size_t index);
size_t tiny_find_object_index(const tiny_node* node, const char* key, size_t klen);
tiny_node* tiny_find_object_value(tiny_node* node, const char* key, size_t klen);
tiny_node* tiny_set_object_key(tiny_node* node, const char* key, size_t klen);
void tiny_remove_object(tiny_node* node, size_t index);

#endif
