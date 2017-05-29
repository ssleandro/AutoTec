/****************************************************************************
 * Title                 :   acquireg_app
 * Filename              :   acquireg_app.h
 * Author                :   Henrique Reis
 * Origin Date           :   29/05/2017
 * Version               :   1.0.0
 * Compiler              :   GCC 5.4 2016q2 / ICCARM 7.40.3.8938
 * Target                :   LPC43XX M4
 * Notes                 :   Qualicode Machine Technologies
 *
 * THIS SOFTWARE IS PROVIDED BY AUTEQ TELEMATICA "AS IS" AND ANY EXPRESSED
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL AUTEQ TELEMATICA OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
/*************** INTERFACE CHANGE LIST **************************************
 *
 *    Date    Version       Author          Description
 * 29/05/2017  1.0.0     Henrique Reis      acquireg_app.h created.
 *
 *****************************************************************************/
#ifndef APP_COMMON_APP_COMMON_ACQUIREG_APP_H_
#define APP_COMMON_APP_COMMON_ACQUIREG_APP_H_

/******************************************************************************
* Includes
*******************************************************************************/

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Configuration Constants
*******************************************************************************/

/******************************************************************************
* Macros
*******************************************************************************/

/******************************************************************************
* Typedefs
*******************************************************************************/

typedef struct {

  //uint8_t  abAdubo[32];      //Adubo por linha
  uint32_t dSomaSem;         //Sementes em todas as Linhas
  uint32_t dDistancia;       //Distância Percorrida  em centimetros
  uint32_t dSegundos;        //Tempo em segundos
  float   fArea;            //Area
  uint32_t adSementes[36];   //Sementes por linha

} tsLinhas;

typedef struct {

  uint32_t dDistancia;      //Distância Percorrida  em centimetros - após config
  uint32_t dSomaSem;       //Soma de sementes parcial de todas as linhas

} tsDistanciaTrab;


typedef struct {

  uint32_t    dTEV;          //Tempo total em excesso de velocidade (em trabalho)
  uint32_t    dMTEV;         //Máximo intervalo de Tempo em Excesso de Velocidade
  float      fVelMax;       //Velocidade Máxima Atingida em excesso de velocidade

} tsVelocidade;

typedef struct {

  tsLinhas  sTrabTotal;     //Acumula valores trabalhando (Total)
  tsLinhas  sTrabTotalDir;  //Acumula valores trabalhando em Arremate do lado direito
  tsLinhas  sTrabTotalEsq;  //Acumula valores trabalhando em Arremate do lado esquerdo

  tsLinhas  sTrabParcial;  //Acumula valores trabalhando (Parcial)
  tsLinhas  sTrabParcDir;  //Acumula valores trabalhando em Arremate do lado direito
  tsLinhas  sTrabParcEsq;  //Acumula valores trabalhando em Arremate do lado esquerdo

  tsLinhas  sManobra;      //Acumula valores manobra (não trabalhando)

  tsLinhas  sTotalReg;     //Acumula valores totais para Registro(trabalhando + manobra)

  tsLinhas  sAvalia;       //Acumula valores em Avaliação (cálculo da média)

  tsDistanciaTrab  sDistTrabTotal;     //Distância parcial - o qual zera após a configuração
  tsDistanciaTrab  sDistTrabTotalEsq;  //Distância parcial - o qual zera após a configuração
  tsDistanciaTrab  sDistTrabTotalDir;  //Distância parcial - o qual zera após a configuração

  tsDistanciaTrab  sDistTrabParcial;     //Distância parcial - o qual zera após a configuração
  tsDistanciaTrab  sDistTrabParcialEsq;  //Distância parcial - o qual zera após a configuração
  tsDistanciaTrab  sDistTrabParcialDir;  //Distância parcial - o qual zera após a configuração


} tsAcumulados;


/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Public Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/

#endif /* APP_COMMON_APP_COMMON_ACQUIREG_APP_H_ */
