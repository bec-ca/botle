
let word_lists = {
  en_ispell_words: "./word_lists/en-ispell-words.dict",
  en_secret_words: "./word_lists/en-secret-words.dict",
  en_wiki_10k: "./word_lists/en-wiki-10k.dict",
  en_wiki_2k: "./word_lists/en-wiki-2k.dict",
  en_wiki_4k: "./word_lists/en-wiki-4k.dict",
  en_words: "./word_lists/en-words.dict",
  pt_secret_words: "./word_lists/pt-secret-words.dict",
  pt_words: "./word_lists/pt-words.dict",
  letreco_words: "./word_lists/letreco-words.dict",
  letreco_secrets: "./word_lists/letreco-secrets.dict",
  pt_xingo: "./word_lists/xingo-words.dict",
  pt_super_senha: "./word_lists/super-senha.dict",
  pt_palavra_do_dia: "./word_lists/palavra-do-dia.dict",
  pt_palavra_do_dia_secrets: "./word_lists/palavra-do-dia-secrets.dict",
}

export function get_allowed_guesses_filename(dict) {
  switch (dict) {
    case 'wordle':
      return word_lists.en_words;
    case 'termoo':
      return word_lists.pt_words;
    case 'letreco':
      return word_lists.letreco_words;
    case 'wiki-2k':
      return word_lists.en_wiki_2k;
    case 'wiki-4k':
      return word_lists.en_wiki_4k;
    case 'wiki-10k':
      return word_lists.en_wiki_10k;
    case 'xingo':
      return word_lists.pt_xingo;
    case 'super-senha':
      return word_lists.pt_super_senha;
    case 'palavra-do-dia':
      return word_lists.pt_palavra_do_dia;
    default:
      throw new Error("Unknown dictionary " + dict);
  }
}

export function get_possible_secrets_filename(dict) {
  switch (dict) {
    case 'wordle':
      return word_lists.en_secret_words;
    case 'termoo':
      return word_lists.pt_secret_words;
    case 'letreco':
      return null;
    case 'wiki-2k':
    case 'wiki-4k':
    case 'wiki-10k':
      return null;
    case 'xingo':
      return null;
    case 'super-senha':
      return null;
    case 'palavra-do-dia':
      return word_lists.pt_palavra_do_dia_secrets;
    default:
      throw new Error("Unknown dictionary " + dict);
  }
}
