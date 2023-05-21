#include "tinyjson.h"
#include <assert.h>  // assert()
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, strtod() */
#include <cstring>    /* memcpy() */

// 定义缓冲区栈的默认大小
#ifndef TINY_PARSE_STACK_INIT_SIZE
#define TINY_PARSE_STACK_INIT_SIZE 256
#endif 

#define EXPECT(c, ch)  do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)  ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)  ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch)    do { *(char*)tiny_context_push(c, sizeof(char)) = (ch); } while(0);

typedef struct {
    const char* json;
    // 栈的相关结构
    char* stack;
    size_t size, top;
} tiny_context;

/// @brief 向 tiny_context 中压入数据
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

/// @brief 从 tiny_context 的 stack 中弹出数据
/// @param c tiny_context
/// @param size 要弹出的数据大小
/// @return 操作后的 top 指针
static void* tiny_context_pop(tiny_context* c, size_t size){
    assert(c->top >= size);  // 确保 stack 中有足够的数据
    return c->stack + (c->top - size); 
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

/// @brief 解析 json 节点中的字符串
/// @param c tiny_context
/// @param node json 节点
/// @return 解析状态码
static int tiny_parse_string(tiny_context* c, tiny_node* node){
    size_t head = c->top, len;
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
                    default:
                        c->top = head;
                        return TINY_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
        case '\0':  // 如果没遇到 \" 而先遇到 \0 则代表字符串不合法（缺少结束的 \"）
            c->top = head;
            return TINY_PARSE_MISS_QUOTATION_MARK;
        default:
            if ((unsigned char)ch < 0x20){
                c->top = head;
                return TINY_PARSE_INVALID_STRING_CHAR;
            }
            PUTC(c, ch);
        }
    }
}

static int tiny_parse_value(tiny_context* c, tiny_node* node){
    switch (*c->json)
    {
    case 'n': return tiny_parse_null(c, node);
    case 't': return tiny_parse_true(c, node);
    case 'f': return tiny_parse_false(c, node);
    default:  return tiny_parse_number(c, node);
    case '"': return tiny_parse_string(c, node);
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
    // 目前仅需处理字符串的空间，因为这部分是我们在堆上申请的，不会自动释放
    if (node->type == TINY_STRING){
        free(node->s.s);
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