#include "tinyjson.h"
#include <assert.h>  // assert()
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, strtod() */
#include <cstring>    /* memcpy() */
#include <cctype>  /* isxdigit() */
#include <string>  /* stoi() */

// 定义缓冲区栈的默认大小
#ifndef TINY_PARSE_STACK_INIT_SIZE
#define TINY_PARSE_STACK_INIT_SIZE 256
#endif 

#define EXPECT(c, ch)  do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)  ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)  ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch)    do { *(char*)tiny_context_push(c, sizeof(char)) = (ch); } while(0);
#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

typedef struct {
    const char* json;
    // 栈的相关结构
    char* stack;
    size_t size, top;
} tiny_context;

/// @brief 向 tiny_context 中压入数据，但实际上并没有做任何 push 的事情，只是在 c 中开辟了足够的空间，完成数据拷贝的还是 memcpy
/// @param c tiny_context
/// @param size 要压入的数据大小
/// @return 操作后的 top 指针
static void* tiny_context_push(tiny_context* c, size_t size){
    void* ret;  // 通用指针类型，使用时要显式地进行类型转换
    assert(size > 0);
    // 若目前的大小不够
    if (c->top + size >= c->size){
        // 若没有分配内存则分配默认大小的内存
        if (c->size == 0){
            c->size = TINY_PARSE_STACK_INIT_SIZE;
        }
        while (c->top + size >= c->size){
            // TODO: 研究扩充倍数与性能的关系

            c->size += c->size >> 1;  // 每次扩充为原来的 1.5 倍
        }
        c->stack = (char*)realloc(c->stack, c->size);  // 重新分配内存
    }    
    ret = c->stack + c->top;  // 即为目前可以插入数据的位置
    c->top += size; 
    return ret;
}

/// @brief 从 tiny_context 的 stack 中弹出数据, 本身不实现 pop 功能，只完成指针的移动
/// @param c tiny_context
/// @param size 要弹出的数据大小
/// @return 操作后的 top 指针
static void* tiny_context_pop(tiny_context* c, size_t size){
    assert(c->top >= size);  // 确保 stack 中有足够的数据
    return c->stack + (c->top -= size);  // 注意这里是 -= ，会更新 top 指针位置
}

static void tiny_parse_whitespace(tiny_context* c){
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'){
        p++;
    }
    c->json = p;
}

static int tiny_parse_null(tiny_context* c, tiny_node* node){
    EXPECT(c, 'n');  // 判断第一个值是否为 n
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l'){
        return TINY_PARSE_INVALID_VALUE;
    }
    c->json += 3;
    node->type = TINY_NULL;
    return TINY_PARSE_OK;  // 解析 null 完成
}

static int tiny_parse_true(tiny_context* c, tiny_node* node){
    EXPECT(c, 't');  // 判断第一个值是否为 t
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e'){
        return TINY_PARSE_INVALID_VALUE;
    }
    c->json += 3;
    node->type = TINY_TRUE;
    return TINY_PARSE_OK;  // 解析 null 完成
}

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

static const char* tiny_parse_hex4(const char* p, unsigned* u) {
    *u = 0;
    for (int i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;  // 左移四位，相当于乘16
        if      (ch >= '0' && ch <= 9) *u |= ch - '0';  // 相当于加
        else if (ch >= 'A' && ch <= 'F') *u |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f') *u |= ch - ('a' - 10);
        else return nullptr;
    }
    return p;
}

static void tiny_encode_utf8(tiny_context* c, unsigned u) {
    // 由于在 PUTC 中做了强转，因此会把 usigned 直接转化为 char
    if (u <= 0x7F) {
        PUTC(c, u & 0xFF);
    }
    else if (u <= 0x7FF) {
        PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
        PUTC(c, 0x80 | ( u       & 0x3F));
    }
    else if (u <= 0xFFFF) {
        PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(c, 0x80 | ((u >> 6)  & 0x3F));
        PUTC(c, 0x80 | ( u        & 0x3F));
    }
    else {
        assert(u <= 0x10FFFF);
        PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >>  6) & 0x3F));
        PUTC(c, 0x80 | ( u        & 0x3F));
    }
}

/// @brief 解析 json 节点中的字符串
/// @param c tiny_context
/// @param node json 节点
/// @return 解析状态码
static int tiny_parse_string(tiny_context* c, tiny_node* node){
    size_t head = c->top, len;
    unsigned u, u2;
    const char* p;
    EXPECT(c, '\"');  // 判断起始字符是否为转义了的引号，这是字符串的起始标识
    p = c->json;
    
    // 无限循环
    for(;;){
        char ch = *p++;  // 注意这里是 p++，先取值再 ++ 
        switch (ch) {
            case '\"':  // 意味着字符串结束
                len = c->top - head;  
                tiny_set_string(node, (const char*)tiny_context_pop(c, len), len);
                c->json = p; 
                return TINY_PARSE_OK;  // 成功解析
            case '\\':
                switch (*p++) {
                    case '\"': PUTC(c, '\"'); break;
                    case '\\': PUTC(c, '\\'); break;
                    case '/':  PUTC(c, '/' ); break;
                    case 'b':  PUTC(c, '\b'); break;
                    case 'f':  PUTC(c, '\f'); break;
                    case 'n':  PUTC(c, '\n'); break;
                    case 'r':  PUTC(c, '\r'); break;
                    case 't':  PUTC(c, '\t'); break;
                    case 'u':
                        if (!(p = tiny_parse_hex4(p, &u)))
                            STRING_ERROR(TINY_PARSE_INVALID_UNICODE_HEX);
                        if (u >= 0xD800 && u <= 0xDBFF) {
                            if (*p++ != '\\') STRING_ERROR(TINY_PARSE_INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')  STRING_ERROR(TINY_PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(p = tiny_parse_hex4(p, &u2))) STRING_ERROR(TINY_PARSE_INVALID_UNICODE_HEX);
                            if (u2 < 0xDC00 || u2 > 0xDFFF) STRING_ERROR(TINY_PARSE_INVALID_UNICODE_SURROGATE);
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        tiny_encode_utf8(c, u);
                        break;
                    default:
                        STRING_ERROR(TINY_PARSE_INVALID_STRING_ESCAPE);
                }
                break;
            case '\0':
                STRING_ERROR(TINY_PARSE_MISS_QUOTATION_MARK);
            default:
                if ((unsigned char)ch < 0x20)
                    STRING_ERROR(TINY_PARSE_INVALID_STRING_CHAR);
                PUTC(c, ch);
        }
    }
}

static int tiny_parse_value(tiny_context* c, tiny_node* node);  // forward declare

static int tiny_parse_array(tiny_context* c, tiny_node* node) {
    size_t size = 0;  // 维护 array 的 size
    int ret;  // 维护返回值
    EXPECT(c, '[');
    tiny_parse_whitespace(c);  // 处理空格
    // 空的 array
    if (*c->json == ']') {
        c->json++;
        node->type = TINY_ARRAY;
        node->a.size = 0;
        return TINY_PARSE_OK;
    }

    for (;;) {
        tiny_node e;
        tiny_init(&e);  // 初始化 json 节点来存放 array 中的 element
        // 逐个解析 array 中的 element，并将信息存在 e 中
        if ((ret = tiny_parse_value(c, &e)) != TINY_PARSE_OK) break;
        // 将解析得到的 array 节点压入栈
        memcpy(tiny_context_push(c, sizeof(tiny_node)), &e, sizeof(tiny_node));
        size++;
        tiny_parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;  // element 的分隔符
            tiny_parse_whitespace(c);
        }
        // array 解析完成
        else if (*c->json == ']') {
            c->json++;
            node->type = TINY_ARRAY;
            node->a.size = size;
            size *= sizeof(tiny_node);
            // 将结果拷贝到当前节点的对应结构中
            memcpy(node->a.e = (tiny_node*)malloc(size), tiny_context_pop(c, size), size); 
        }
        // 在 array 中解析完一个 element 只可能遇到 , 或 ]; 其余均为无效情况
        else {
            ret = TINY_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }

    for (size_t i = 0; i < size; i++) {
        tiny_free((tiny_node*)tiny_context_pop(c, sizeof(tiny_node)));
    }
    return ret;
}


static int tiny_parse_value(tiny_context* c, tiny_node* node){
    switch (*c->json)
    {
    case 'n' : return tiny_parse_null(c, node);
    case 't' : return tiny_parse_true(c, node);
    case 'f' : return tiny_parse_false(c, node);
    default  : return tiny_parse_number(c, node);
    case '"' : return tiny_parse_string(c, node);
    case '[' : return tiny_parse_array(c, node);
    case '\0': return TINY_PARSE_EXPECT_VALUE;  // '\0' 代表字符串结束符，因此可以判断是否为空
    }
}

int tiny_parse(tiny_node* node, const char* json){
    tiny_context c;
    int ret;
    assert(node != nullptr);
    
    c.json = json;  // 存储数据
    c.stack = nullptr;
    c.size = c.top = 0;  // 初始化栈结构

    tiny_init(node);  // 初始化节点
    tiny_parse_whitespace(&c);  // 处理空格
    if ((ret = tiny_parse_value(&c, node)) == TINY_PARSE_OK){
        tiny_parse_whitespace(&c);  // 处理字符串后空格
        if (*c.json != '\0'){  // 处理完后指向的不是字符串的结束符，即后面还有其他字符
            ret = TINY_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);  // 正常处理后栈应该弹空
    free(c.stack);  // 释放栈空间
    return ret;
}

void tiny_free(tiny_node* node){
    assert(node != nullptr);
    switch (node->type) {
    case TINY_STRING:
        free(node->s.s);
        break;
    case TINY_ARRAY:
        // 先释放每个 ele 开辟的空间
        for (size_t i = 0; i < node->a.size; i++){
            tiny_free(&node->a.e[i]);
        }
        free(node->a.e);  // 最后释放自己开辟的空间
        break;
    default: break;
    }
    node->type = TINY_NULL;
}

tiny_type tiny_get_type(const tiny_node* node){
    assert(node != nullptr);  // 先判断是否为空指针
    return node->type;
}

int tiny_get_boolean(const tiny_node* node){
    assert(node != nullptr && (node->type == TINY_FALSE || node->type == TINY_TRUE));
    return node->type == TINY_TRUE;  // 巧妙！
}

void tiny_set_boolean(tiny_node* node, int b){
    assert(node != nullptr);
    tiny_free(node);
    node->type = b ? TINY_TRUE : TINY_FALSE;  // 巧妙！
}

double tiny_get_number(const tiny_node* node){
    assert(node != nullptr && node->type == TINY_NUMBER);
    return node->n;
}

void tiny_set_number(tiny_node* node, double n){
    assert(node != nullptr);
    tiny_free(node);
    node->n = n;
    node->type = TINY_NUMBER;
}

const char* tiny_get_string(const tiny_node* node){
    assert(node != nullptr && node->type == TINY_STRING);
    return node->s.s;
}

size_t tiny_get_string_length(const tiny_node* node){
    assert(node != nullptr && node->type == TINY_STRING);
    return node->s.len;
}

/// @brief 设置 json 节点中的值为指定字符串
/// @param node json 节点
/// @param s 字符串指针
/// @param len 字符串长度
void tiny_set_string(tiny_node* node, const char* s, size_t len){
    // (s != nullptr || len == 0) 的意思是当且仅当 s 指针不为空但 len 不为零时返回 false
    assert(node != nullptr && (s != nullptr || len == 0));
    tiny_free(node);
    node->s.s = (char*)malloc(len + 1);  // 由于要放 '\0' 字符
    memcpy(node->s.s, s, len);  // 进行字节级别的拷贝
    node->s.s[len] = '\0';
    node->s.len = len;
    node->type = TINY_STRING;
}

/// @brief 获取数组元素个数
/// @param node 数组（json节点）
/// @return 数组元素个数
size_t tiny_get_array_size(const tiny_node* node) {
    assert(node != nullptr && node->type == TINY_ARRAY);
    return node->a.size;
}

/// @brief 获取数组元素
/// @param node 数组（json节点）
/// @param index 下标
/// @return 对应下表数组元素
tiny_node* tiny_get_array_element(const tiny_node* node, size_t index) {
    assert(node != nullptr && node->type == TINY_ARRAY);
    assert(index < node->a.size);
    return &node->a.e[index];
}