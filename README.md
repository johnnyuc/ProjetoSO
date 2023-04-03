# Menu
- [Menu](#menu)
- [Coisas por fazer](#coisas-por-fazer)
- [Coisas feitas](#coisas-feitas)
---
<br>

> Aqui criamos um sistema operativo. Neste momento tem sistema, mas não opera

> NOTA: O dump é uma pasta para testes mas não para lixo, deixar organizado

# Coisas por fazer
- sys_manager.c:
  - Se o ficheiro ficar de grandes dimensões poderá ser útil dividir em mais pequenos, como por exemplo um ficheiro loader.c, um ficheiro logger.c, etc. Para já estou a colocar tudo no mesmo arquivo (self-contained)
  - Output das inicializações tem de ser escrito no log e ainda não está
 
---
<br>

# Coisas feitas
  - Makefile configurado, e creio que seja o mais completo possível para o desenvolvimento posterior do trabalho
  - config_loader():
    - Inicialização do programa com as configurações passadas. O ficheiro de configurações é dado via terminal: $ home_iot {ficheiro de configuração}
  - log_writer():
    - Já escreve num ficheiro, definido no header file como constante, e inclui o timestamp
    - O envio sincronizado de output significa que o programa irá escrever tanto no ficheiro de log como no ecrã (stdout) ao mesmo tempo e na mesma ordem. Ou seja, cada vez que uma mensagem é escrita no ecrã ela também será escrita no arquivo de log e em ambos os casos, a ordem de escrita será a mesma. Isso garante que o conteúdo do arquivo de log seja uma réplica exata do que foi mostrado no ecrã. Para isso usamos um mutex que tranca qualquer acesso à função enquanto ela não terminar de escrever no ficheiro e no ecrã, por ordem.
  - main_initializer():
    - Inicilização dos named pipes (não são pedidos para a fase intermédia).
---