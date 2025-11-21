# GS Iot

## Nome dos Participantes
- Rafael Ali Oliveira Khalil 561240
- Bruno Lisboa Rezende 562228


## 🔗 Link para o Projeto no Wokwi

[Projeto no Wokwi](https://wokwi.com/projects/448190777870670849)

## 🔗 Link para o Projeto no Youtube

[Projeto no Youtube](https://youtu.be/bI8yf7IAcYE?si=XYyl2t_DwP82yxPM)

<img width="525" height="497" alt="image" src="https://github.com/user-attachments/assets/224ec3d1-d728-477f-8a52-ec38c56817ac" />

## Descrição do Problema

Longas horas de trabalho sentado podem causar problemas posturais graves, incluindo dores crônicas na coluna, LER/DORT e outros problemas de saúde ocupacional. A falta de feedback imediato sobre má postura contribui para que trabalhadores mantenham posições inadequadas por períodos prolongados.

## Solução Proposta

Sistema de monitoramento de postura em tempo real utilizando ESP32 e sensor MPU6050. O dispositivo é instalado no encosto da cadeira e monitora continuamente a inclinação do usuário, fornecendo alertas visuais e sonoros quando detecta má postura por período prolongado.

## O que o Sistema Faz

- Monitora a inclinação do encosto da cadeira continuamente
- Classifica a postura em três níveis: Boa Postura, Atenção e Má Postura
- Aciona LED vermelho e buzzer após 10 segundos de má postura
- Permite silenciar o alerta pressionando um botão
- Envia dados para ThingSpeak a cada 15 segundos para monitoramento remoto

## Componentes Utilizados

- ESP32 DevKit V1
- Sensor MPU6050
- LED vermelho
- Buzzer
- Botão push-button

## Aplicação em Saúde Ocupacional

- Prevenção de lesões por má postura
- Feedback em tempo real para correção postural
- Análise de padrões comportamentais através dos dados históricos
- Possibilidade de monitoramento corporativo de ergonomia

