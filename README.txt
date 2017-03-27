Processo de autentificação com os keyfiles:

Cliente A ao conectar-se a B tem de:
    -quando insere o comando "connect", tem de inserir no campo "keyfile" o 
     seu próprio nome de modo a abrir o ficheiro com o seu nome. Tem de 
     existir um ficheiro com o seu nome completo na pasta "keyfile" existente
     na pasta do programa.

Cliente B ao aceitar a conecção de A tem de:
    -abrir o ficheiro com o nome do cliente que se está a tentar conectar.
     O nome completo do utilizador que se está a tentar conectar é enviado numa 
     mensagem de protocolo automática ( este processo é realizado pelo
     programa e o utilizador não tem de se preocupar com o mesmo ). Tem de 
     existir um ficheiro com o nome completo do utilizador que se está a tentar
     conectar na pasta "keyfile" existente na pasta do programa.

Exemplo passo a passo:

-Afonso Costa quer conectar-se a Bruno Gonçalves
-Afonso Costa cria um ficheiro na pasta "keyfile" com nome : "AfonsoCosta.txt"
-Bruno Gonçalves tem de ter esse mesmo ficheiro com o mesmo nome na sua pasta
 "keyfile"
-Afonso Costa insere o seguinte comando no schat: 
                               "connect Bruno.Gonçalves AfonsoCosta"
-A autentificação será completada e os utilizadores poderão trocar mensagens