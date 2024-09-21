
# Challenge John Deere - DEV4INNOVATION
## Sumário

- [Introdução](#introdução)
- [Desenvolvimento](#desenvolvimento)
  - [Arquitetura](#arquitetura)
  - [Estrutura Banco de Dados](#estrutura-banco-de-dados)
  - [Tecnologias](#tecnologias)
- [Funcionalidades](#funcionalidades)
  - [Rebocador](#rebocador)
  - [Administrador](#administrador)
- [Resultados](#resultados)
  - [Exemplos](#exemplos)
- [Código Fonte e Estrutura](#codigo-fonte-e-estrutura)
- [Autores](#autores)
- [Instalação](#react-+-typeScript-+-vite)
- [Stacks](#stacks)



## Introdução
O projeto busca melhorar a visibilidade e o controle das atividades de logística dentro da fábrica, otimizando o tempo e reduzindo erros no processo de entrega.
![Proposta do sistema](./src/assets/proposta.png)

## Desenvolvimento
### Arquitetura

A arquitetura do sistema é baseada em uma solução IoT para localização e controle de um rebocador dentro de uma fábrica. Utilizamos um ESP32 para calcular a posição do rebocador, e essas coordenadas são enviadas para uma API desenvolvida em Node.js, que armazena os dados no banco de dados MongoDB. O front-end, construído em Vite.js com TypeScript, permite que o administrador visualize o andamento das entregas e o desempenho dos rebocadores.

- O ESP32 coleta dados de localização via RSSI dos roteadores presentes na fábrica.
- O ESP32 envia os dados para uma API REST (Node.js) via Wi-Fi.
- A API processa os dados e os armazena no banco de dados MongoDB.
- O front-end consome esses dados e os exibe em um mapa, permitindo que o administrador veja a posição atual e as tarefas em andamento do rebocador.

![Arquitetura do Sistema](./src/assets/arquitetura.png)
![Diagrama de Caso de Uso](./src/assets/diagrama.png)

### Estrutura Banco de Dados
![Estrutura Banco de Dados](./src/assets/banco-de-dados.jpg)

### Tecnologias

As principais tecnologias envolvidas no projeto são:

- **Vite.js**: Framework para desenvolvimento front-end, escolhido pela sua rapidez e facilidade de configuração com TypeScript.
- **Node.js**: Servidor back-end que gerencia as requisições do ESP32 e do front-end.
- **MongoDB**: Banco de dados NoSQL utilizado para armazenar as coordenadas e informações do rebocador.
- **ESP32**: Dispositivo IoT responsável por calcular a posição do rebocador e enviar os dados.
- **TypeScript**: Usado no desenvolvimento do front-end e back-end, para maior consistência e manutenção do código.
- **C++**: Linguagem utilizada para programar o ESP32.

## Funcionalidades
### Rebocador
- Controle de entregas realizadas
- Direcionamento de entregas a serem feitas
### Administrador
- Controle das informações dos rebocadores
- Informações sobre o desempenho das atividades

## Resultados
- Aplicativo Web voltado para o rebocador para auxiliar na locomoção de peças dentro da fábrica
- Aplicativo Web voltado para o administrador para visualizar o andamento das atividades realizadas pelos rebocadores

### Exemplos
[**Inserir print do painel do administrador**]
[**Inserir print da interface de monitoramento do rebocador**]

## Código Fonte e Estrutura
- O código-fonte do projeto está disponível na pasta `scr`.
- As imagens utilizadas no projeto estão organizadas na pasta `assets`.
- Link do vídeo demonstrativo: [YouTube](https://link-do-video)

## Autores

- [Ana Luiza Oliveira Dourado](https://www.linkedin.com/in/ana-dourado/)
- [Lucas Rodrigues Grecco](https://www.linkedin.com/in/lucasrgrecco/)
- [Monique Ferreira dos Anjos](https://www.linkedin.com/in/ferreira-monique/)
- [Felipe Wapf Fettback](https://github.com/FelipeFettback)
- [Ronaldo Veloso Filho](https://www.linkedin.com/in/ronaldoveloso/)

## Stacks
- [BACK-END - API](https://github.com/An4lu/DeereGO-Back)
- [FRONT-END - WEB APP](https://github.com/An4lu/DeereGO)
- [ESP](https://github.com/lucvs07/deereGO-ESP)


# React + TypeScript + Vite

This template provides a minimal setup to get React working in Vite with HMR and some ESLint rules.

Currently, two official plugins are available:

- [@vitejs/plugin-react](https://github.com/vitejs/vite-plugin-react/blob/main/packages/plugin-react/README.md) uses [Babel](https://babeljs.io/) for Fast Refresh
- [@vitejs/plugin-react-swc](https://github.com/vitejs/vite-plugin-react-swc) uses [SWC](https://swc.rs/) for Fast Refresh

## Expanding the ESLint configuration

If you are developing a production application, we recommend updating the configuration to enable type aware lint rules:

- Configure the top-level `parserOptions` property like this:

```js
export default tseslint.config({
  languageOptions: {
    // other options...
    parserOptions: {
      project: ['./tsconfig.node.json', './tsconfig.app.json'],
      tsconfigRootDir: import.meta.dirname,
    },
  },
})
```

- Replace `tseslint.configs.recommended` to `tseslint.configs.recommendedTypeChecked` or `tseslint.configs.strictTypeChecked`
- Optionally add `...tseslint.configs.stylisticTypeChecked`
- Install [eslint-plugin-react](https://github.com/jsx-eslint/eslint-plugin-react) and update the config:

```js
// eslint.config.js
import react from 'eslint-plugin-react'

export default tseslint.config({
  // Set the react version
  settings: { react: { version: '18.3' } },
  plugins: {
    // Add the react plugin
    react,
  },
  rules: {
    // other rules...
    // Enable its recommended rules
    ...react.configs.recommended.rules,
    ...react.configs['jsx-runtime'].rules,
  },
})
```
