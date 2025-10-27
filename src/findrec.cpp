//
// Created by luizg on 10/25/25.
//

#include <iostream>

#include "artigo/artigo.h"
#include "file_manager/FileManager.h"

int main(int argc, char *argv[]) {
    FileManager fm(1000, 4, "data.dat");

    Artigo art;

    fm.buscarRegistro(atoi(argv[1]), art);

    art.imprimir();

    return 0;
}
