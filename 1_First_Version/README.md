# Primeira Versão

A primeira versão deve corresponder a um servidor capaz de tratar conexões concorrentes (vários clientes simultaneamente), seguindo o protocolo HTTP 1.0 (conexões não persistentes).

## Observações

Nessa primeira versão ainda não foi utilizada a biblioteca pthreads.h para permitir que o servidor trate de forma concorrente vários clientes. Até o momento, o servidor só trata requisições de GET.

O código foi desenvolvido em ambiente linux, utilizando o ubuntu 20.04 LTS. 
De forma a rodar o servidor, basta compilar o arquivo server.c (gcc server.c -o <nome do executavel>) e executá-lo em modo super usuário (sudo ./<executavel>). A porta definida no arquivo pode ser modificada, e toda vez deve-se recompilar o arquivo.
* Para acessar o servidor, basta entrar no navegador e utilizar o localhost:porta/arquivo.tipo (Exemplo: porta 8080, arquivo image.jpg || localhost:8080/image.jpg) 

## Afirmação

***“Este projeto foi desenvolvido integralmente pela equipe, sem ajuda não autorizada de alunos não membros do projeto no processo de codificação”.***

## Referências

https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa

https://trungams.github.io/2020-08-23-a-simple-http-server-from-scratch/

## Informação sobre o grupo

**Universidade:** Universidade Federal do Pampa (UNIPAMPA).

**Disciplina:** Redes de Computadores.

**Professor:** Leonardo Bidese de Pinho.

**Integrantes do Grupo:** Lucas Vilanova Barcellos, Renato Sayyed de Souza e Willian Domingues.
