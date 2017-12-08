// DOGE LANGUAGE
/* noun      : "LISP" | "language" | "book";
 * adjective : "wow" | "many" | "so" | "such";
 * phrase    : <adjective> <noun>;
 * doge      : <phrase>*;
 */


// Decimals
/* nonzero : '1' | '2' | '3' | '4' \
 *         | '5' | '6' | '7' | '8' | '9';
 * zero    : '0';
 * digit   : <nonzero> | <zero>;
 * integer : <zero> | (<nonzero> <digit>*);
 * decimal : <integer> '.' <digit>* <nonzero>;
 */


// Web URLs
/* scheme     : "http" | "https";
 * alphabets  : 'a' | 'b' | ...;
 * identifier : "com" | "edu" | "org" | ...;
 *
 * subdomain  : (<alphabets> '.')*;
 * domain     : <alphabets>+;
 * domain_id  : ('.' <identifier>)+;
 * path       : ('/' <alphabets>)*;
 * url        : <scheme> "://" <subdomain> <domain> <domain_id> <path>;
 */


// English
/* object   : (<article> | ) <adjective>* <noun>;
 * sentence : <object> <verb> <proposition> <object> '.'
 */
