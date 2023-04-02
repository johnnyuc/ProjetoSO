- [Coisas por fazer](#coisas-por-fazer)

# Coisas por fazer
- sys_manager.c:
  - Fazer um log_writer(), que tem de incluir timestamp
  - Se o ficheiro ficar de grandes dimensões poderá ser útil dividir em mais pequenos, como por exemplo um ficheiro loader.c, um ficheiro logger.c, etc.
  - config_loader():
    - O output tem que ir parar ao log.txt via log_writer()
  - log_writer():
    - O envio sincronizado de output significa que o programa irá escrever tanto no ficheiro de log como no ecrã (stdout) ao mesmo tempo e na mesma ordem. Ou seja, cada vez que uma mensagem é escrita no ecrã ela também será escrita no arquivo de log e em ambos os casos, a ordem de escrita será a mesma. Isso garante que o conteúdo do arquivo de log seja uma réplica exata do que foi mostrado no ecrã. Temos que usar mutexes para dar locks aos acessos do ficheiro de log e escrita no ecrã.