
let word_lists = {
  en_ispell_words: "./word_lists/en-ispell-words.dict",
  en_secret_words: "./word_lists/en-secret-words.dict",
  en_wiki_10k: "./word_lists/en-wiki-10k.dict",
  en_wiki_2k: "./word_lists/en-wiki-2k.dict",
  en_wiki_4k: "./word_lists/en-wiki-4k.dict",
  en_words: "./word_lists/en-words.dict",
  pt_secret_words: "./word_lists/pt-secret-words.dict",
  pt_words: "./word_lists/pt-words.dict",
  pt_xingo: "./word_lists/xingo-words.dict",
}

export function get_allowed_guesses_filename(dict) {
  switch (dict) {
    case 'wordle':
    case 'wordle-secret':
      return word_lists.en_words;
    case 'termoo':
    case 'termoo-secrets':
      return word_lists.pt_words;
    case 'wiki-2k':
      return word_lists.en_wiki_2k;
    case 'wiki-4k':
      return word_lists.en_wiki_4k;
    case 'wiki-10k':
      return word_lists.en_wiki_10k;
    case 'xingo':
      return word_lists.pt_xingo;
    default:
      throw new Error("Bug");
  }
}

export function get_possible_secrets_filename(dict) {
  switch (dict) {
    case 'wordle':
      return null;
    case 'wordle-secret':
      return word_lists.en_secret_words;
    case 'termoo':
      return null;
    case 'termoo-secrets':
      return word_lists.pt_secret_words;
    case 'wiki-2k':
    case 'wiki-4k':
    case 'wiki-10k':
      return null;
    case 'xingo':
      return null;
    default:
      throw new Error("Bug");
  }
}
