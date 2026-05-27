#include <iostream>
#include <string>
#include <cassert>

struct Transformer;
struct Number;
struct BinaryOperation;
struct FunctionCall;
struct Variable;

struct Expression {
    virtual ~Expression() {}

    virtual double evaluate() const = 0;
    virtual Expression* transform(Transformer* tr) const = 0;
};

struct Transformer { // реализация паттерна Visitor 
    virtual ~Transformer() {}

    virtual Expression* transformNumber(Number const*) = 0;
    virtual Expression* transformBinaryOperation(BinaryOperation const*) = 0;
    virtual Expression* transformFunctionCall(FunctionCall const*) = 0;
    virtual Expression* transformVariable(Variable const*) = 0;
};

struct Number : Expression {
    Number(double value) : value_(value) {};

    double value() const {
        return value_;
    }
    double evaluate() const {
        return value_;
    }
    Expression* transform(Transformer* tr) const {
        return tr->transformNumber(this);
    }
private:
    double value_;
};

struct BinaryOperation : Expression {
    enum {
        PLUS = '+',
        MINUS = '-',
        DIV = '/',
        MUL = '*'
    };

    BinaryOperation(Expression const* left, int op, Expression const* right)
    : left_(left), op_(op), right_(right) {
        assert(left_ && right_);
    }
    ~BinaryOperation() {
        delete left_;
        delete right_;
    }

    double evaluate() const {
        double left = left_->evaluate(); // вычисляем левую часть
        double right = right_->evaluate(); // вычисляем правую часть
        switch (op_) { // в зависимости от вида операции, складываем, вычитаем, умножаем или делим левую и правую части
        case PLUS: return left + right;
        case MINUS: return left - right;
        case DIV: return left / right;
        case MUL: return left * right;
        }
    }
    Expression* transform(Transformer* tr) const {
        return tr->transformBinaryOperation(this);
    }
    Expression const* left() const {
        return left_;
    }
    Expression const* right() const {
        return right_;
    }

    int operation() const {
        return op_;
    }

private:
    Expression const* left_;
    Expression const* right_;
    int op_;
};

struct FunctionCall : Expression {
    FunctionCall(std::string const& name, Expression const* arg)
        : name_(name), arg_(arg) {
        assert(arg_);
        assert(name_ == "sqrt" || name_ == "abs");
    }
    ~FunctionCall() {
        delete arg_;
    }

    double evaluate() const {
        if (name_ == "sqrt") {
            return std::sqrt(arg_->evaluate());
        }
        else {
            return std::fabs(arg_->evaluate());
        }
    }

    Expression* transform(Transformer* tr) const {
        return tr->transformFunctionCall(this);
    }

    std::string const& name() const {
        return name_;
    }
    Expression const* arg() const {
        return arg_;
    }

private:
    std::string const name_;
    Expression const* arg_;
};

struct Variable : Expression {
    Variable(std::string const name)
        : name_(name) { }

    std::string const& name() const {
        return name_;
    }
    double evaluate() const {
        return 0.0;
    }
    Expression* transform(Transformer* tr) const {
        return tr->transformVariable(this);
    }

private:
    std::string const name_;
};


struct CopySyntaxTree : Transformer {
    Expression* transformNumber(Number const* number) {
        return new Number(number->value()); // получаем значение старого числа через метод value и создаем новое число с этим значением
    }

    Expression* transformBinaryOperation(BinaryOperation const* binop) {
        Expression* newLeft = binop->left()->transform(this); // получаем левый операнд и копируем его
        Expression* newRight = binop->right()->transform(this); // получаем правый операнд и копируем его
        return new BinaryOperation(newLeft, binop->operation(), newRight); // создаем новый объект с новыми операндами
    }

    Expression* transformFunctionCall(FunctionCall const* fcall) {
        Expression* newArg = fcall->arg()->transform(this); // получаем старый аргумент функции и копируем его
        return new FunctionCall(fcall->name(), newArg); // создаем новый объект с новым аргументом
    }

    Expression* transformVariable(Variable const* var) {
        return new Variable(var->name()); // получаем имя старой переменной и создаем новую переменную с этим именем

    }
};

struct FoldConstants : Transformer {
    Expression* transformNumber(Number const* number) {
        return new Number(number->value()); // число уже является константой, просто создаём его копию
    }

    Expression* transformBinaryOperation(BinaryOperation const* binop) {
        Expression* newLeft = binop->left()->transform(this); // сворачиваем левое поддерево
        Expression* newRight = binop->right()->transform(this); // сворачиваем правое поддерево

        Number* leftNumber = dynamic_cast<Number*>(newLeft); // проверяем стали ли оба операнда числами
        Number* rightNumber = dynamic_cast<Number*>(newRight);

        if (leftNumber && rightNumber) {
            double left = leftNumber->value(); // если оба операнда - числа, операцию можно вычислить
            double right = rightNumber->value();
            double result = 0.0;

            switch (binop->operation()) {
            case BinaryOperation::PLUS:
                result = left + right;
                break;

            case BinaryOperation::MINUS:
                result = left - right;
                break;

            case BinaryOperation::DIV:
                result = left / right;
                break;

            case BinaryOperation::MUL:
                result = left * right;
                break;
            }

            delete newLeft;
            delete newRight;

            return new Number(result);
        }

        return new BinaryOperation(newLeft, binop->operation(), newRight);
    }

    Expression* transformFunctionCall(FunctionCall const* fcall) {
        Expression* newArg = fcall->arg()->transform(this); // сворачиваем аргумент функции

        Number* argNumber = dynamic_cast<Number*>(newArg); // проверяем является ли аргумент числом

        if (argNumber) {
            double arg = argNumber->value(); // если аргумент - число, функцию можно вычислить
            double result = 0.0;

            if (fcall->name() == "sqrt") {
                result = std::sqrt(arg);
            }
            else {
                result = std::fabs(arg);
            }

            delete newArg;

            return new Number(result);
        }

        return new FunctionCall(fcall->name(), newArg);
    }

    Expression* transformVariable(Variable const* var) {
        return new Variable(var->name()); // копируем переменную, так как значение неизвестно
    }
};

int main() {
    Number* n32 = new Number(32.0);
    Number* n16 = new Number(16.0);

    BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
    FunctionCall* callSqrt = new FunctionCall("sqrt", minus);

    Variable* var = new Variable("var");

    BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL,callSqrt);
    FunctionCall* callAbs = new FunctionCall("abs", mult);

    FoldConstants FC;
    Expression* newExpr = callAbs->transform(&FC);

    std::cout << newExpr->evaluate(); //  0
}