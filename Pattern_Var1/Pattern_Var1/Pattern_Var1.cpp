//Используя паттерн проектирования Visitor составьте структуру
//классов Персона →(Студент, Профессор), Visitor →(Сессия, Каникулы), в которой есть
//следующие виды деятельности : в Сессию Студент сдает экзамены, а Профессор принимает
//экзамены, соответственно, в Каникулы Студент отдыхает, Профессор уходит в отпуск.
#include <iostream>
#include <string>

struct Visitor;
struct Student;
struct Professor;


struct Person { // базовый класс для всех персон
    virtual ~Person() {}
    
    virtual void accept(Visitor* visitor) const = 0; // метод принимает Visitor и передаёт ему текущий объект
};


struct Visitor { // Базовый интерфейс Visitor
    virtual ~Visitor() {}
   
    virtual void visitStudent(Student const* student) = 0; // отдельный метод для обработки студента
   
    virtual void visitProfessor(Professor const* professor) = 0; // отдельный метод для обработки профессора
};


struct Student : Person { // студент
    Student(std::string const& name)
        : name_(name) { }

    std::string const& name() const {
        return name_;
    }

    void accept(Visitor* visitor) const {
        visitor->visitStudent(this);
    }

private:
    std::string name_;
};


struct Professor : Person { // профессор
    Professor(std::string const& name)
        : name_(name) { }

    std::string const& name() const {
        return name_;
    }

    void accept(Visitor* visitor) const {
        visitor->visitProfessor(this);
    }

private:
    std::string name_;
};


struct Session : Visitor { // Visitor для сессии
    void visitStudent(Student const* student) {
        std::cout << "В сессию студент " << student->name() << " сдает экзамены." << std::endl;
    }

    void visitProfessor(Professor const* professor) {
        std::cout << "В сессию профессор " << professor->name()  << " принимает экзамены." << std::endl;
    }
};


struct Vacation : Visitor { // Visitor для каникул
    void visitStudent(Student const* student) {
        std::cout << "В каникулы студент " << student->name() << " отдыхает." << std::endl;
    }

    void visitProfessor(Professor const* professor) {
        std::cout << "В каникулы профессор " << professor->name() << " уходит в отпуск." << std::endl;
    }
};

int main() {
    setlocale(LC_ALL, "Rus");
    Student student("Иванов");
    Professor professor("Петров");

    Session session;
    Vacation vacation;

    Person* p1 = &student;
    Person* p2 = &professor;

    p1->accept(&session); // В сессию студент Иванов сдает экзамены
    p2->accept(&session); // В сессию профессор Петров принимает экзамены
    
    std::cout << std::endl;

    p1->accept(&vacation); // В каникулы студент Иванов отдыхает
    p2->accept(&vacation); // В каникулы профессор Петров уходит в отпуск
}
