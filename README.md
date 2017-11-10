# Release Notes

## v0.1.006

    a.  Adicionado RTC device

    b.  Adicionado DS2411R device

    c.	Leitura e armazenamento do ID number do equipamento pelo modulo Control.

    d.  Sincronização do device RTC com a hora do GPS atraves do modulo Control.

    e.  Arrumado a sincronização da leitura de sementes com a sinalização de um metro percorrido.

    f.  Corrigido bug na sinalização de alarmes para as linhas acima da linha 32.

    g.  Alteração na device da CAN.

    h.  Alteração na mcu GPIO.

    i.  Adicionado rotina de formatação da memória.

    j.  Adicionado rotina de leitura do status da formatação da memória.

    k.  Adicionado rotina de leitura das informações do sistema de arquivos.


## v0.1.005
## 1.	GUI / Isobus

    a.  Adicionado suporte a restransmissão no envio do object pool.

    b.  Adicionado suporte a idioma on-line (inglês, português e espanhol).

    c.	Adicionado suporte a unidade de medida na inicialização.

    d.  Adicionado suporte a unidade de medida on-line.

    e.  Alteração das strings dos limites de entrada de dados conforme a unidade de medida.

    f.  Alteração dos limites de entrada de dados conforme a unidade de medida (input number).

    g.  Adicionado objetos para a apresentação da lista de sensores object pool.

    h.  Compilação na versão release funcional.


## v0.1.004
## 1.	GUI / Isobus

    a.	Adicionado alarme para média fora da tolerância (2 beeps)

    b.	Adicionado alarme para nenhuma semente na linha (Beep continuo)

    c.	Adicionado alarme para excesso de velocidade (Beep continuo)

    d.	Adicionado indicação visual para alarmes nas linhas de plantio.

    e.	Adicionado senha para acesso as configurações.

    f.	Adicionado opção para alteração de senha.

    g.	Adicionado suporte a troca de sensores.

    h.	Adicionado estatísticas da CAN na tela sistema.

    i.  Adicionado suporte a idioma na inicialização do sistema (inglês, português e espanhol).

    j.	Corrigido bug na instalação, após mudança na configuração ou quando a instalação era apagada.

    k.  Corrigido bug ao alternar entre modo monitor de area e modo plantadeira.

    l.  Corrigido alinhamento do texto nas soft key do object pool.



## v0.1.003
## 1.	GUI / Isobus

    a.	Adicionado suporte a arremate

    b.	Adicionado suporte a monitor de àrea

    c.	Adicionado à tela de configuração suporte para monitor de àrea. Ao selecionar monitor de àrea sim, o sistema retorna para a tela monitor de àrea.

    d.	Adicionado novas soft key masks para o modo monitor de àrea.

    e.	Adicionado alarmes no momento da instalaçãpo (beep quanto um sensor é instalado)

    f.	Corrigido bug no popup de velocidade (imprecisão na velocidade apresentada)

    g.	Corrigido bug ao alternar linhas (bloqueio de acesso a linhas errado)

    h.	Corrigido bug em ignorar linha (ao cancelar ignorar linha ocorria um erro ao atualizar a tela plantadeira)

    i.	Corrigido bug na tela confirmar alterações de configuração (ao modificar certas configurações zera os totais, ao modificar o número de linhas zera a instalação)

    j.	Corrigido bug na instalação, após mudança na configuração ou quando a instalação era apagada

## 2.	GPS

    a.	Corrigido bug na configuração do GPS

    b.	Corrigido bug na recepção das mensagens do GPS