#ifndef TINYJSON_H__
#define TINYJSON_H__

// 定义 json 中的 7 种数据类型（bool 中的 true 和 false 分开）
typedef enum { TINY_NULL, TINY_FALSE, TINY_TRUE, TINY_NUMBER, TINY_STRING, 
    TINY_ARRAY, TINY_OBJECT } tiny_type;

// 定义 json 中的节点结构 tiny_node，节点中包含 tiny_type 类型的值
// 教程中名称是 _value，但我总觉得变扭，改用 node
typedef struct {
    tiny_type type;
}tiny_node;

/// @brief 枚举类型，代表解析状态码
enum {
    // 赋值了第一个变量为 0，后面的值会依次递增
    TINY_PARSE_OK = 0,  // 正确解析
    TINY_PARSE_EXPECT_VALUE,  // 节点值为空白 1
    TINY_PARSE_INVALID_VALUE,  // 节点中有非法的值 2
    TINY_PARSE_ROOT_NOT_SINGULAR  // 一个值之后除了空白还有其他字符 3
};

/// @brief 解析 json 节点中的值并返回int类型的解析状态码
/// @param node json 节点
/// @param json 该节点中的字符串类型的值
/// @return 解析状态码
int tiny_parse(tiny_node* node, const char* json);

/// @brief 返回解析出的节点中的数据类型
/// @param node json 节点
/// @return json 数据类型
tiny_type tiny_get_type(const tiny_node* node);


#endif
