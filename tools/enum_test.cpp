#include <logger.h>


enum test {
    cock = 3,
    balls = 2,
    dick = 1,
};

int main(){
    test s = cock;
    logger::info("1: {}", s);
    test * k = &s;
    logger::info("2: {}", *k);
    int * j = (int *) k;
    logger::info("3: {}", *j);
    s = balls;
    logger::info("1: {}", s);
    logger::info("2: {}", *k);
    logger::info("3: {}", *j);
    s = dick;
    logger::info("1: {}", s);
    logger::info("2: {}", *k);
    logger::info("3: {}", *j);
}