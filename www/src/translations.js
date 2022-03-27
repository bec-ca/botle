let pt = {
  'choose-words': "Escolha um conjunto de palavras:",
  'hard-mode': "Modo difícil",
  'start': 'Começar',
  'loading': 'Iniciando...',
  'selected-guess': 'Palavra escolhida',
  'instructions': 'Digite a palavra escolhida a cima no wordle e escolha o padrão de cores que apareceu no wordle abaixo. Uma palavra diferente pode ser escolhida no lado direito.',
  'dont-use-secrets': 'Não use a lista de segredos separada. (Isso dificulta achar a palavra secreta)',
  'back': 'Voltar',
  'word': 'Chute',
  'avg': 'Media',
  'worst': 'Pior',
  'depth': 'Profundidade',
  'done': 'Pronto',
  'thinking done': 'Analise completa',
  'initializing': 'Iniciando...',
  'thinking': 'Analizando...',
}

let en = {
  'choose-words': "Choose a set of words:",
  'hard-mode': "Hard mode",
  'start': 'Start',
  'loading': 'Loading...',
  'selected-guess': 'Selected guess',
  'instructions': 'Type in the selected word above on wordle and pick the color pattern that you got bellow. You can also choose a different word on the right side.',
  'dont-use-secrets': 'Don\'t use separate list of secrets. (This makes it harder to guess)',
  'back': 'Back',
  'word': 'Word',
  'avg': 'Avg',
  'worst': 'Worst',
  'depth': 'Depth',
  'done': 'Done',
  'thinking done': 'Thinking done',
  'initializing': 'Initializing...',
  'thinking': 'Thinking...',
}

let languages = {
  'en': en,
  'pt': pt,
}

let default_language = en;

function find_language() {
  let language_name = navigator.language.split('-')[0];
  console.log('Detected language is ', language_name);
  return languages[language_name] || languages['en'];
}

let language = find_language();

export default function get_text(key) {
  return language[key] || default_language[key] || key;
}
