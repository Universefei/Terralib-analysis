Nota de Instalação no Linux para a Versão 3.2.0 da TerraLib
===========================================================



1. Descompacte o arquivo (.tar.gz) contendo
os scripts de instalação em um diretório apropriado,
como por exemplo:
/root/terraphpinstall.

2. Os seguintes arquivos (scripts) deverão
ser encontrados neste diretório:
- te_main_install;
- te_install_terralib;
- te_install_terramanager;
- te_install_terraphp;


3. Ajuste o privilégio de execução para
os arquivos de script se necessário (chmod 700 te_*).

4. Você deverá estar logado como usuário
root para realizar a instalação.

5. Executando o comando:
./te_main_install

será exibida a seguinte tela:

"
Selecione uma da opcoes de instalacao abaixo (ou digite 0 para sair):

1 => Instalar TerraLib
2 => Instalar TerraManager
3 => Instalar TerraPHP
0 => Sair 
"

6. Este script permite que você instale a TerraLib
e a extensão TerraPHP.

7. Para instalar a extensão TerraPHP,
você precisará ter instalado os seguintes
softwares em seu sistema:
- biblioteca gd (gd-2.0.33)
- Apache (1.3.X ou superior)
- PHP 4.3.11 (ou qualquer outro da série 4.4.x) 
- Biblioteca MySQL client (libmysqlclient) se for instalar com
suporte a MySQL

8. caso da instalação do TerraManager,
que é utilizado pela extensão TerraPHP,
é necessário indicar quais os drivers de bancos
de dados deverão estar presentes na extensão TerraPHP.
Edite o arquivo te_install_terramanager e
configure a seguinte variável:
DATABASE_2_USE="HAS_POSTGRESQL"

Caso deseje instalar outros drivers,
pode-se usar espaço para separar uma
das seguintes opções:
HAS_POSTGRESQL, HAS_OCI, HAS_MYSQL.

Ex: DATABASE_2_USE="HAS_POSTGRESQL HAS MYSQL"

8. Após configurar o script de instalação
do TerraManager (te_install_terramanager),
podemos iniciar a instalação dos módulos TerraLib.

9. Possivelmente, você vai querer gerar
a biblioteca TerraLib em modo release
para obter o maior desempenho possível no TerraPHP.
Sendo assim, edite os arquivos ".pro"
dentro da pasta terralibx e substitua
os modificadores "debug" por "release"
nestes arquivos. Depois, utilize o qmake
para gerar os novos Makefiles.
Ex:
qmake terralib.pro
qmake shapelib.pro

10. Inicie a instalação (./te_main_install).

11. Dúvidas ou sugestões, entre em contato com a equipe TerraLib (http://www.terralib.org).


Obs.:

1. Nós recomendamos que a instalação do TerraPHP seja
feita como um módulo externo. Isso facilita a
manutenção e upgrade da extensão.

2. Durante a instalação do TerraPHP, se você optou
por instalar o PHP e o script de configuração do PHP
não foi executado com sucesso, verifique
se o pacote do lex (ou flex) está instalado no seu sistema.
