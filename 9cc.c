#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>

// トークンの型を表す値
enum {
	TK_NUM = 256, // 整数トークン
	TK_EOF,
	TK_EQ, // equal
	TK_NE, // not equal
	TK_LE, // less than or equal
	TK_GE, // greater than or equal
};

// ノードの型を表す値
enum {
	ND_NUM = 256,
};

 // トークンの型
typedef struct {
	int ty; // トークンの型
	int val; // tyがTK_NUMの場合、その数値
	char *input; // トークン文字列(エラーメッセージ用)
} Token;

// 可変長ベクタ
typedef struct {
	void **data;
	int capacity;
	int len;
} Vector;

// 新しい可変長ベクタを作成する
Vector *new_vector() {
	Vector *vec = malloc(sizeof(Vector));
	vec->data = malloc(sizeof(void *) * 16);
	vec->capacity = 16;
	vec->len = 0;
	return vec;
}

// 可変長ベクタに新しい要素を追加する
void vec_push(Vector *vec, void *elem) {
	if (vec->capacity == vec ->len) {
		vec->capacity *= 2;
		vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
	}
	vec->data[vec->len++] = elem;
}

// トークンを格納する可変長ベクタ
Vector *token_vec;

// 可変長ベクタからi番目のトークンを取り出し
Token get_token(Vector *vec, int i){
	return (* (Token *)vec->data[i]);
}

// 可変長ベクタにトークンを追加
void add_token(int ty, char* p, int i){
	if (i >token_vec->len){
		return;
	}
	Token *atoken = malloc(sizeof(Token));
	atoken->ty = ty;
	atoken->input = p;
	vec_push(token_vec, (void *)atoken);
}

// 可変長ベクタに数字トークンを追加
void add_num_token(int val, char* p, int i){
	if (i > token_vec->len){
		return;
	}
	Token *atoken = malloc(sizeof(Token));
	atoken->ty = TK_NUM;
	atoken->input = p;
	atoken->val = val;
	vec_push(token_vec, (void *)atoken);
}

// データ構造のユニットテスト
int expect(int line, int expected, int actual) {
	if (expected == actual)
		return 1;
	fprintf(stderr, "%d: %d expected, but got %d\n",line, expected, actual);
	exit(1);
}

void runtest() {
	Vector *vec = new_vector();
	expect(__LINE__, 0, vec->len);

	for (int i = 0; i < 100; i++)
		vec_push(vec, (void *)(__intptr_t)i);

	expect(__LINE__, 100, vec->len);
	expect(__LINE__, 0, (long)vec->data[0]);
	expect(__LINE__, 50, (long)vec->data[50]);
	expect(__LINE__, 99, (long)vec->data[99]);

	printf("OK\n");
}

// エラーを報告するための関数
// printfと同じ引数をとる
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// ノードの型
typedef struct Node {
	int ty;
	struct Node *lhs;
	struct Node *rhs;
	int val;
}Node;

// 新しいノードを作成する
Node *new_node(int ty, Node *lhs, Node *rhs) {
	Node *node = malloc(sizeof(Node));
	node->ty = ty;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = malloc(sizeof(Node));
	node->ty = ND_NUM;
	node->val = val;
	return node;
}

// 現在着目しているトークンのインデックス
int pos = 0;

// 次のトークンが引数と等しい場合にトークンを読み進めて真を返す
int consume(int ty) {
	Token tk = get_token(token_vec, pos);
	if (tk.ty != ty)
		return 0;
	pos++;
	return 1;
}

Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *term();
Node *unary();

// 等号
Node *equality() {
	Node *node = relational();

	for(;;){
		if (get_token(token_vec, pos).ty == TK_EQ){
			pos++;
			node = new_node(TK_EQ, node, relational());
		}else if (get_token(token_vec, pos).ty == TK_NE){
			pos++;
			node = new_node(TK_NE, node, relational());
		}else{
			return node;
		}
	}
}

//不等号
Node *relational(){
	Node *node = add();

	for(;;){
		if (get_token(token_vec, pos).ty == TK_LE){
			pos++;
			node = new_node(TK_LE, node, add());
		}else if (get_token(token_vec, pos).ty == TK_GE){
			pos++;
			node = new_node(TK_LE, add(), node);
		}else if (consume('<')){
			node = new_node('<', node, add());
		}else if (consume('>')){
			node = new_node('<', add(), node);
		}else{
			return node;
		}
	}
}

// 和算
Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume('+'))
			node = new_node('+', node, mul());
		else if (consume('-'))
			node = new_node('-', node, mul());
		else
			return node;
	}
}

// 乗算
Node *mul() {
	Node *node = unary();
	
	for (;;) {
		if (consume('*'))
			node = new_node('*', node, unary());
		else if (consume('/'))
			node = new_node('/', node, unary());
		else
			return node;
	}
}

// 単項プラスマイナス
Node *unary(){
	if (consume('+'))
		return term();
	if (consume('-'))
		return new_node('-', new_node_num(0), term());
	return term();
}

// カッコ
Node *term() {
	if (consume('(')) {
		Node *node = add();
		if (!consume(')'))
			error("開きカッコに対する閉じカッコがありません： %s", get_token(token_vec, pos).input);
		return node;
	}

	if (get_token(token_vec, pos).ty == TK_NUM)
		return new_node_num(get_token(token_vec, pos++).val);

	error("数値でも開きカッコでもないトークンです： %s", get_token(token_vec, pos).input);
}


// pがさしている文字列をトークンに分割して可変長ベクタに保存する
void tokenize (char *p){
	token_vec = new_vector();
	int i = 0;
	while (*p){
		if (isspace(*p)){
			p++;
			continue;
		}
		if (!strncmp(p, "==", 2)){
			add_token(TK_EQ, p, i);
			i++;
			p = p + 2;
			continue;
		}
		if (!strncmp(p, "!=", 2)){
			add_token(TK_NE, p, i);
			i++;
			p = p + 2;
			continue;
		}
		if (!strncmp(p, "<=", 2)){
			add_token(TK_LE, p, i);
			i++;
			p = p + 2;
			continue;
		}
		if (!strncmp(p, ">=", 2)){
			add_token(TK_GE, p, i);
			i++;
			p = p + 2;
			continue;
		}
		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || 
				*p == '(' || *p == ')' || *p == '<' || *p == '>'){
			add_token(*p, p, i);
			i++;
			p++;
			continue;
		}
	
		if (isdigit(*p)){
			add_num_token(strtol(p, &p, 10), p, i);
			i++;
			continue;
		}
	
		error("トークナイズできません： %s", p);
		exit(1);
	}
	add_token(TK_EOF, p, i);
}

//スタック操作命令
void gen(Node *node){
	if (node->ty == ND_NUM) {
		printf("	push %d\n", node->val);
		return;
	}
	gen(node->lhs);
	gen(node->rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");

	switch (node->ty) {
		case TK_EQ:
			printf("	cmp rax, rdi\n");
			printf("	sete al\n");
			printf("	movzb rax, al\n");
			break;
		case TK_NE:
			printf("	cmp rax, rdi\n");
			printf("	setne al\n");
			printf("	movzb rax, al\n");
			break;
		case TK_LE:
			printf("	cmp rax, rdi\n");
			printf("	setle al\n");
			printf("	movzb rax, al\n");
			break;
		case '<':
			printf("	cmp rax, rdi\n");
			printf("	setl al\n");
			printf("	movzb rax, al\n");
			break;
		case '+':
			printf("	add rax, rdi\n");
			break;
		case '-':
			printf("	sub rax, rdi\n");
			break;
		case '*':
			printf("	mul rdi\n");
			break;
		case '/':
			printf("	mov rdx, 0\n");
			printf("	div rdi\n");
	}
	printf("	push rax\n");
}


int main(int argc, char **argv){
	if (!strcmp(argv[1], "-test")) {
		runtest();
		return 0;
	}

	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません。\n");
		return 1;
	}

	// トークナイズしてパースする
	tokenize(argv[1]);
	Node *node = equality();

	
	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// 抽象構文木を下りながらコード生成
	gen(node);

	// スタックトップに敷き全体の値が残っているはずなので
	// それをRAXにロードしてから関数からの返り値とする
	printf("	pop rax\n");
	printf("	ret\n");
	return 0;
}
