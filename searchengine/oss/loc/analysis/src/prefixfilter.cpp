/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/

#include "prefixfilter.h"

#include "Clucene.h"

namespace analysis {

    using namespace lucene::analysis;
    using namespace lucene::util;

    const wchar_t* HebrewPrefixes[] = {
       L"\x05d0",  // aleph
       L"\x05d1",  // bet
       L"\x05d4",  // he
       L"\x05d5",  // vav
       L"\x05d9",  // yud
       L"\x05db",  // kaf 
       L"\x05dc",  // lamed
       L"\x05de",  // mem 
       L"\x05e0",  // nun 
       L"\x05e9",  // shin 
       L"\x05ea",  // tav
       NULL
    };

    PrefixFilter::PrefixFilter(TokenStream* input, bool deleteTs, const wchar_t** prefixes) 
    : TokenFilter(input, deleteTs), prefixes_(prefixes), token_(), prefixFound_( false ) {}
    
    bool PrefixFilter::next(Token* token) {
		// 1. Either use reduced form of last token or fetch a new token
		if ( prefixFound_ ) {
			token->set(token_.termText(),
					   token_.startOffset(), 
					   token_.endOffset(), 
					   token_.type());
			token->setPositionIncrement(0);
			prefixFound_ = false;
		} else if (!input->next( token )) {
			// no token found
			return false;
		}
		
		// 2. Try to match prefixes with the token
		const wchar_t* text = token->_termText;
		int sz = 0; 
		for (int i = 0; prefixes_[i] && !sz; i++) {
			for (; prefixes_[i][sz]; sz++) {
				if (text[sz] != prefixes_[i][sz]) {
					sz = 0;
					break;
				}
			}
		}
		// 3. If prefix found, cut prefix and store cut form of token,
		//    if cut form is non-empty
		if ( sz && token->termTextLength() > sz) {
			token_.set(token->termText() + sz,
					   token->startOffset(), 
					   token->endOffset(), 
					   token->type());
			prefixFound_ = true; 
		}
		return true; 
    }
    
#define APOSTROPHE1 L'\''
#define APOSTROPHE2 L'\x2019'
    
    const wchar_t* FrenchArticles[] = {
    	L"l",
    	L"m",
    	L"t",
    	L"qu",
    	L"n",
    	L"s",
    	L"j",
    	NULL
    };

    // This list should not be relied on
    // Consult language experts!
    const wchar_t* ItalianArticles[] = {
    	L"l", 		// the
    	L"d", 		// from
    	L"un", 		// one, a
    	L"dell", 	// to
    	L"all",	
    	L"e",
    	L"quest",
    	L"quell",
    	L"buon", 	// means good, should it be here?
    	NULL
    };

    ElisionFilter::ElisionFilter(TokenStream* input, bool deleteTs, const wchar_t** articles) 
    : TokenFilter(input, deleteTs), articles_(articles) {}
    
    bool ElisionFilter::next(Token* token) {
    	if ( input->next(token) ) {
			wchar_t* text = token->_termText;
			int i = 0;
			for (; text[i]; i++) {
				if ( text[i] == APOSTROPHE1 
				  || text[i] == APOSTROPHE2 ) {
					break; 
				}
			}
			if ( text[i] ) {
				for (int j = 0; articles_[j]; j++) {
					if ( memcmp( articles_[j], text, i ) ) {
						for (int k = 0; ; k++) {
							text[k] = text[k+i+1];
							if (!text[k+i+1]) break; // end of string 
						}
						token->resetTermTextLen();
						return true; 
					}
				}
			}
			return true; 
    	}
    	return false; 
    }
    
	using namespace lucene::analysis::standard; 

	TokenStream* HebrewAnalyzer::tokenStream(const wchar_t* fieldName, Reader* reader) {
		auto_ptr<TokenStream> ret(  new StandardTokenizer(reader) ); 
		
		ret.reset( new LowerCaseFilter( ret.release(), true ) ); 
		ret.reset( new StandardFilter( ret.release(), true ) ); 
		ret.reset( new PrefixFilter( ret.release(), true, HebrewPrefixes ) ); 
		
		return ret.release();  	
	}

	TokenStream* HebrewQueryAnalyzer::tokenStream(const wchar_t* fieldName, Reader* reader) {
		auto_ptr<TokenStream> ret(  new StandardTokenizer(reader) ); 
		
		ret.reset( new LowerCaseFilter( ret.release(), true ) ); 
		ret.reset( new StandardFilter( ret.release(), true ) ); 
		
		return ret.release();  	
	}

	TokenStream* FrenchAnalyzer::tokenStream(const wchar_t* fieldName, Reader* reader) {
		auto_ptr<TokenStream> ret(  new StandardTokenizer(reader) ); 
		
		ret.reset( new LowerCaseFilter( ret.release(), true ) ); 
		ret.reset( new StandardFilter( ret.release(), true ) ); 
		ret.reset( new ElisionFilter( ret.release(), true, FrenchArticles ) ); 
		
		return ret.release();  	
	}
	
	const TCHAR* NonEnglishStopWords::FRENCH_STOP_WORDS[] = {
	  _T("a"), _T("afin"), _T("ai"), _T("ainsi"), _T("après"), _T("attendu"), _T("au"), _T("aujourd"),
	  _T("auquel"), _T("aussi"), _T("autre"), _T("autres"), _T("aux"), _T("auxquelles"), _T("auxquels"),
	  _T("avait"), _T("avant"), _T("avec"), _T("avoir"), _T("c"), _T("car"), _T("ce"), _T("ceci"), _T("cela"), _T("celle"), _T("celles"), _T("celui"), _T("cependant"), _T("certain"),
	  _T("certaine"), _T("certaines"), _T("certains"), _T("ces"), _T("cet"), _T("cette"), _T("ceux"), _T("chez"), _T("ci"),
	  _T("combien"), _T("comme"), _T("comment"), _T("concernant"), _T("contre"), _T("d"), _T("dans"), _T("de"), _T("debout"),
	  _T("dedans"), _T("dehors"), _T("delà"), _T("depuis"), _T("derrière"), _T("des"), _T("désormais"), _T("desquelles"),
	  _T("desquels"), _T("dessous"), _T("dessus"), _T("devant"), _T("devers"), _T("devra"), _T("divers"), _T("diverse"),
	  _T("diverses"), _T("doit"), _T("donc"), _T("dont"), _T("du"), _T("duquel"), _T("durant"), _T("dès"), _T("elle"), _T("elles"),
	  _T("en"), _T("entre"), _T("environ"), _T("est"), _T("et"), _T("etc"), _T("etre"), _T("eu"), _T("eux"), _T("excepté"), _T("hormis"),
	  _T("hors"), _T("hélas"), _T("hui"), _T("il"), _T("ils"), _T("j"), _T("je"), _T("jusqu"), _T("jusque"), _T("l"), _T("la"), _T("laquelle"),
	  _T("le"), _T("lequel"), _T("les"), _T("lesquelles"), _T("lesquels"), _T("leur"), _T("leurs"), _T("lorsque"), _T("lui"), _T("là"),
	  _T("ma"), _T("mais"), _T("malgré"), _T("me"), _T("merci"), _T("mes"), _T("mien"), _T("mienne"), _T("miennes"), _T("miens"), _T("moi"),
	  _T("moins"), _T("mon"), _T("moyennant"), _T("même"), _T("mêmes"), _T("n"), _T("ne"), _T("ni"), _T("non"), _T("nos"), _T("notre"),
	  _T("nous"), _T("néanmoins"), _T("nôtre"), _T("nôtres"), _T("on"), _T("ont"), _T("ou"), _T("outre"), _T("où"), _T("par"), _T("parmi"),
	  _T("partant"), _T("pas"), _T("passé"), _T("pendant"), _T("plein"), _T("plus"), _T("plusieurs"), _T("pour"), _T("pourquoi"),
	  _T("proche"), _T("près"), _T("puisque"), _T("qu"), _T("quand"), _T("que"), _T("quel"), _T("quelle"), _T("quelles"), _T("quels"),
	  _T("qui"), _T("quoi"), _T("quoique"), _T("revoici"), _T("revoilà"), _T("s"), _T("sa"), _T("sans"), _T("sauf"), _T("se"), _T("selon"),
	  _T("seront"), _T("ses"), _T("si"), _T("sien"), _T("sienne"), _T("siennes"), _T("siens"), _T("sinon"), _T("soi"), _T("soit"),
	  _T("son"), _T("sont"), _T("sous"), _T("suivant"), _T("sur"), _T("ta"), _T("te"), _T("tes"), _T("tien"), _T("tienne"), _T("tiennes"),
	  _T("tiens"), _T("toi"), _T("ton"), _T("tous"), _T("tout"), _T("toute"), _T("toutes"), _T("tu"), _T("un"), _T("une"), _T("va"), _T("vers"),
	  _T("voici"), _T("voilà"), _T("vos"), _T("votre"), _T("vous"), _T("vu"), _T("vôtre"), _T("vôtres"), _T("y"), _T("à"), _T("ça"), _T("ès"),
	  _T("été"), _T("être"), _T("ô"), NULL
	};

	const TCHAR* NonEnglishStopWords::BRAZILIAN_STOP_WORDS[] = {
   _T("a"),_T("ainda"),_T("alem"), _T("ambas"), _T("ambos"), _T("antes"),
   _T("ao"), _T("aonde"), _T("aos"), _T("apos"), _T("aquele"), _T("aqueles"),
   _T("as"), _T("assim"), _T("com"), _T("como"), _T("contra"), _T("contudo"),
   _T("cuja"), _T("cujas"), _T("cujo"), _T("cujos"), _T("da"), _T("das"), _T("de"),
   _T("dela"), _T("dele"), _T("deles"), _T("demais"), _T("depois"), _T("desde"),
   _T("desta"), _T("deste"), _T("dispoe"), _T("dispoem"), _T("diversa"),
   _T("diversas"), _T("diversos"), _T("do"), _T("dos"), _T("durante"), _T("e"),
   _T("ela"), _T("elas"), _T("ele"), _T("eles"), _T("em"), _T("entao"), _T("entre"),
   _T("essa"), _T("essas"), _T("esse"), _T("esses"), _T("esta"), _T("estas"),
   _T("este"), _T("estes"), _T("ha"), _T("isso"), _T("isto"), _T("logo"), _T("mais"),
   _T("mas"), _T("mediante"), _T("menos"), _T("mesma"), _T("mesmas"), _T("mesmo"),
   _T("mesmos"), _T("na"), _T("nas"), _T("nao"), _T("nas"), _T("nem"), _T("nesse"), _T("neste"),
   _T("nos"), _T("o"), _T("os"), _T("ou"), _T("outra"), _T("outras"), _T("outro"), _T("outros"),
   _T("pelas"), _T("pelas"), _T("pelo"), _T("pelos"), _T("perante"), _T("pois"), _T("por"),
   _T("porque"), _T("portanto"), _T("proprio"), _T("propios"), _T("quais"), _T("qual"),
   _T("qualquer"), _T("quando"), _T("quanto"), _T("que"), _T("quem"), _T("quer"),_T("se"),
   _T("seja"), _T("sem"), _T("sendo"), _T("seu"), _T("seus"), _T("sob"), _T("sobre"), _T("sua"),
   _T("suas"), _T("tal"), _T("tambem"), _T("teu"), _T("teus"), _T("toda"), _T("todas"), _T("todo"),
   _T("todos"), _T("tua"), _T("tuas"), _T("tudo"), _T("um"), _T("uma"), _T("umas"), _T("uns"),
   NULL
	};
   
   const TCHAR* NonEnglishStopWords::CZECH_STOP_WORDS[] = {
    _T("a"),_T("s"),_T("k"),_T("o"),_T("i"),_T("u"),_T("v"), _T("z"),_T("dnes"),
    _T("cz"),_T("t\u00edmto"),_T("bude\u0161"),_T("budem"), _T("byli"), _T("jse\u0161"), _T("m\u016fj"), _T("sv\u00fdm"), _T("ta"), _T("tomto"), _T("tohle"), _T("tuto"), _T("tyto"),
    _T("jej"), _T("zda"), _T("pro\u010d"), _T("m\u00e1te"), _T("tato"), _T("kam"), _T("tohoto"), _T("kdo"), _T("kte\u0159\u00ed"),
    _T("mi"), _T("n\u00e1m"), _T("tom"), _T("tomuto"), _T("m\u00edt"), _T("nic"), _T("proto"), _T("kterou"), _T("byla"),
    _T("toho"), _T("proto\u017ee"), _T("asi"), _T("ho"), _T("na\u0161i"), _T("napi\u0161te"), _T("re"), _T("co\u017e"), _T("t\u00edm"),
    _T("tak\u017ee"), _T("sv\u00fdch"), _T("jej\u00ed"), _T("sv\u00fdmi"), _T("jste"), _T("aj"), _T("tu"), _T("tedy"), _T("teto"),
    _T("bylo"), _T("kde"), _T("ke"), _T("prav\u00e9"), _T("ji"), _T("nad"), _T("nejsou"), _T("\u010di"), _T("pod"), _T("t\u00e9ma"),
    _T("mezi"), _T("p\u0159es"), _T("ty"), _T("pak"), _T("v\u00e1m"), _T("ani"), _T("kdy\u017e"), _T("v\u0161ak"), _T("neg"), _T("jsem"),
    _T("tento"), _T("\u010dl\u00e1nku"), _T("\u010dl\u00e1nky"), _T("aby"), _T("jsme"), _T("p\u0159ed"), _T("pta"), _T("jejich"),
    _T("byl"), _T("je\u0161t\u011b"), _T("a\u017e"), _T("bez"), _T("tak\u00e9"), _T("pouze"), _T("prvn\u00ed"), _T("va\u0161e"), _T("kter\u00e1"),
    _T("n\u00e1s"), _T("nov\u00fd"), _T("tipy"), _T("pokud"), _T("m\u016f\u017ee"), _T("strana"), _T("jeho"), _T("sv\u00e9"), _T("jin\u00e9"),
    _T("zpr\u00e1vy"), _T("nov\u00e9"), _T("nen\u00ed"), _T("v\u00e1s"), _T("jen"), _T("podle"), _T("zde"), _T("u\u017e"), _T("b\u00fdt"), _T("v\u00edce"),
    _T("bude"), _T("ji\u017e"), _T("ne\u017e"), _T("kter\u00fd"), _T("by"), _T("kter\u00e9"), _T("co"), _T("nebo"), _T("ten"), _T("tak"),
    _T("m\u00e1"), _T("p\u0159i"), _T("od"), _T("po"), _T("jsou"), _T("jak"), _T("dal\u0161\u00ed"), _T("ale"), _T("si"), _T("se"), _T("ve"),
    _T("to"), _T("jako"), _T("za"), _T("zp\u011bt"), _T("ze"), _T("do"), _T("pro"), _T("je"), _T("na"), _T("atd"), _T("atp"),
    _T("jakmile"), _T("p\u0159i\u010dem\u017e"), _T("j\u00e1"), _T("on"), _T("ona"), _T("ono"), _T("oni"), _T("ony"), _T("my"), _T("vy"),
    _T( "j\u00ed"), _T("ji"), _T("m\u011b"), _T("mne"), _T("jemu"), _T("tomu"), _T("t\u011bm"), _T("t\u011bmu"), _T("n\u011bmu"), _T("n\u011bmu\u017e"),
    _T("jeho\u017e"), _T("j\u00ed\u017e"), _T("jeliko\u017e"), _T("je\u017e"), _T("jako\u017e"), _T("na\u010de\u017e"),
    NULL
    };
   
   const TCHAR* NonEnglishStopWords::GERMAN_STOP_WORDS[] = {
  _T("einer"), _T( "eine"), _T( "eines"), _T( "einem"), _T( "einen"),
  _T("der"), _T( "die"), _T( "das"), _T( "dass"), _T( "daß"),
  _T("du"), _T( "er"), _T( "sie"), _T( "es"),
  _T("was"), _T( "wer"), _T( "wie"), _T( "wir"),
  _T("und"), _T( "oder"), _T( "ohne"), _T( "mit"),
  _T("am"), _T( "im"), _T( "in"), _T( "aus"), _T( "auf"),
  _T("ist"), _T( "sein"), _T( "war"), _T( "wird"),
  _T("ihr"), _T( "ihre"), _T( "ihres"),
  _T("als"), _T( "für"), _T( "von"), _T( "mit"),
  _T("dich"), _T( "dir"), _T( "mich"), _T( "mir"),
  _T("mein"), _T( "sein"), _T( "kein"),
  _T("durch"), _T( "wegen"), _T( "wird"),
  NULL 
};

   const TCHAR* NonEnglishStopWords::GREEK_STOP_WORDS[] = {
    _T( "ο"  ),  _T(  "η" ), _T( "το" ), _T( "οι" ), _T( "τα" ), _T( "του" ), _T( "τησ" ), _T( "των" ), _T( "τον" ), _T( "την" ), _T( "και"  ),
    _T( "κι" ), _T( "κ" ), _T( "ειμαι" ), _T( "εισαι" ), _T( "ειναι" ), _T( "ειμαστε" ), _T( "ειστε" ), _T( "στο" ), _T( "στον"  ),
    _T( "στη" ), _T( "στην" ), _T( "μα" ), _T( "αλλα" ), _T( "απο" ), _T( "για" ), _T( "π�?οσ" ), _T( "με" ), _T( "σε" ), _T( "ωσ"  ),
    _T( "πα�?α" ), _T( "αντι" ), _T( "κατα" ), _T( "μετα" ), _T( "θα" ), _T( "να" ), _T( "δε" ), _T( "δεν" ), _T( "μη" ), _T( "μην"  ),
    _T( "επι" ), _T( "ενω" ), _T( "εαν" ), _T( "αν" ), _T( "τοτε" ), _T( "που" ), _T( "πωσ" ), _T( "ποιοσ" ), _T( "ποια" ), _T( "ποιο"  ),
    _T( "ποιοι" ), _T( "ποιεσ" ), _T( "ποιων" ), _T( "ποιουσ" ), _T( "αυτοσ" ), _T( "αυτη" ), _T( "αυτο" ), _T( "αυτοι"  ),
    _T( "αυτων" ), _T( "αυτουσ" ), _T( "αυτεσ" ), _T( "αυτα" ), _T( "εκεινοσ" ), _T( "εκεινη" ), _T( "εκεινο"  ),
    _T( "εκεινοι" ), _T( "εκεινεσ" ), _T( "εκεινα" ), _T( "εκεινων" ), _T( "εκεινουσ" ), _T( "οπωσ" ), _T( "ομωσ"  ),
    _T( "ισωσ" ), _T( "οσο" ), _T( "οτι" ),
    NULL
};

   const TCHAR* NonEnglishStopWords::DUTCH_STOP_WORDS[] = {
      _T("de"), _T("en"), _T("van"), _T("ik"), _T("te"), _T("dat"), _T("die"), _T("in"), _T("een"),
       _T("hij"), _T("het"), _T("niet"), _T("zijn"), _T("is"), _T("was"), _T("op"), _T("aan"), _T("met"), _T("als"), _T("voor"), _T("had"),
       _T("er"), _T("maar"), _T("om"), _T("hem"), _T("dan"), _T("zou"), _T("of"), _T("wat"), _T("mijn"), _T("men"), _T("dit"), _T("zo"),
       _T("door"), _T("over"), _T("ze"), _T("zich"), _T("bij"), _T("ook"), _T("tot"), _T("je"), _T("mij"), _T("uit"), _T("der"), _T("daar"),
       _T("haar"), _T("naar"), _T("heb"), _T("hoe"), _T("heeft"), _T("hebben"), _T("deze"), _T("u"), _T("want"), _T("nog"), _T("zal"),
       _T("me"), _T("zij"), _T("nu"), _T("ge"), _T("geen"), _T("omdat"), _T("iets"), _T("worden"), _T("toch"), _T("al"), _T("waren"),
       _T("veel"), _T("meer"), _T("doen"), _T("toen"), _T("moet"), _T("ben"), _T("zonder"), _T("kan"), _T("hun"), _T("dus"),
       _T("alles"), _T("onder"), _T("ja"), _T("eens"), _T("hier"), _T("wie"), _T("werd"), _T("altijd"), _T("doch"), _T("wordt"),
       _T("wezen"), _T("kunnen"), _T("ons"), _T("zelf"), _T("tegen"), _T("na"), _T("reeds"), _T("wil"), _T("kon"), _T("niets"),
       _T("uw"), _T("iemand"), _T("geweest"), _T("andere"),
      NULL      
    };

   const TCHAR* NonEnglishStopWords::RUSSIAN_STOP_WORDS[] = {
  _T("а"), _T("без"), _T("более"), _T("бы"), _T("был"), _T("была"), _T("были"), _T("было"), _T("быть"), _T("в"),
   _T("вам"), _T("вас"), _T("весь"), _T("во"), _T("вот"), _T("все"), _T("всего"), _T("всех"), _T("вы"), _T("где"), _T(
   "да"), _T("даже"), _T("для"), _T("до"), _T("его"), _T("ее"), _T("ей"), _T("ею"), _T("если"), _T("есть"), _T(
   "еще"), _T("же"), _T("за"), _T("здесь"), _T("и"), _T("из"), _T("или"), _T("им"), _T("их"), _T("к"), _T("как"),
   _T("ко"), _T("когда"), _T("кто"), _T("ли"), _T("либо"), _T("мне"), _T("может"), _T("мы"), _T("на"), _T("надо"), _T(
   "наш"), _T("не"), _T("него"), _T("нее"), _T("нет"), _T("ни"), _T("них"), _T("но"), _T("ну"), _T("о"), _T("об"), _T(
   "однако"), _T("он"), _T("она"), _T("они"), _T("оно"), _T("от"), _T("очень"), _T("по"), _T("под"), _T("при"), _T(
   "с"), _T("со"), _T("так"), _T("также"), _T("такой"), _T("там"), _T("те"), _T("тем"), _T("то"), _T("того"), _T(
   "тоже"), _T("той"), _T("только"), _T("том"), _T("ты"), _T("у"), _T("уже"), _T("хотя"), _T("чего"), _T("чей"), _T(
   "чем"), _T("что"), _T("чтобы"), _T("чье"), _T("чья"), _T("эта"), _T("эти"), _T("это"), _T("я"),
  NULL
};

   const TCHAR* NonEnglishStopWords::EXTENDED_ENGLISH_STOP_WORDS[] = {
   _T("a"), _T("about"), _T("above"), _T("across"), _T("adj"), _T("after"), _T("afterwards"),
         _T("again"), _T("against"), _T("albeit"), _T("all"), _T("almost"), _T("alone"), _T("along"),
         _T("already"), _T("also"), _T("although"), _T("always"), _T("among"), _T("amongst"), _T("an"),
         _T("and"), _T("another"), _T("any"), _T("anyhow"), _T("anyone"), _T("anything"),
         _T("anywhere"), _T("are"), _T("around"), _T("as"), _T("at"), _T("be"), _T("became"), _T("because"),
         _T("become"), _T("becomes"), _T("becoming"), _T("been"), _T("before"), _T("beforehand"),
         _T("behind"), _T("being"), _T("below"), _T("beside"), _T("besides"), _T("between"),
         _T("beyond"), _T("both"), _T("but"), _T("by"), _T("can"), _T("cannot"), _T("co"), _T("could"),
         _T("down"), _T("during"), _T("each"), _T("eg"), _T("either"), _T("else"), _T("elsewhere"),
         _T("enough"), _T("etc"), _T("even"), _T("ever"), _T("every"), _T("everyone"), _T("everything"),
         _T("everywhere"), _T("except"), _T("few"), _T("first"), _T("for"), _T("former"),
         _T("formerly"), _T("from"), _T("further"), _T("had"), _T("has"), _T("have"), _T("he"), _T("hence"),
         _T("her"), _T("here"), _T("hereafter"), _T("hereby"), _T("herein"), _T("hereupon"), _T("hers"),
         _T("herself"), _T("him"), _T("himself"), _T("his"), _T("how"), _T("however"), _T("i"), _T("ie"), _T("if"),
         _T("in"), _T("inc"), _T("indeed"), _T("into"), _T("is"), _T("it"), _T("its"), _T("itself"), _T("last"),
         _T("latter"), _T("latterly"), _T("least"), _T("less"), _T("ltd"), _T("many"), _T("may"), _T("me"),
         _T("meanwhile"), _T("might"), _T("more"), _T("moreover"), _T("most"), _T("mostly"), _T("much"),
         _T("must"), _T("my"), _T("myself"), _T("namely"), _T("neither"), _T("never"),
         _T("nevertheless"), _T("next"), _T("no"), _T("nobody"), _T("none"), _T("noone"), _T("nor"),
         _T("not"), _T("nothing"), _T("now"), _T("nowhere"), _T("of"), _T("off"), _T("often"), _T("on"),
         _T("once one"), _T("only"), _T("onto"), _T("or"), _T("other"), _T("others"), _T("otherwise"),
         _T("our"), _T("ours"), _T("ourselves"), _T("out"), _T("over"), _T("own"), _T("per"), _T("perhaps"),
         _T("rather"), _T("s"), _T("same"), _T("seem"), _T("seemed"), _T("seeming"), _T("seems"),
         _T("several"), _T("she"), _T("should"), _T("since"), _T("so"), _T("some"), _T("somehow"),
         _T("someone"), _T("something"), _T("sometime"), _T("sometimes"), _T("somewhere"),
         _T("still"), _T("such"), _T("t"), _T("than"), _T("that"), _T("the"), _T("their"), _T("them"),
         _T("themselves"), _T("then"), _T("thence"), _T("there"), _T("thereafter"), _T("thereby"),
         _T("therefor"), _T("therein"), _T("thereupon"), _T("these"), _T("they"), _T("this"),
         _T("those"), _T("though"), _T("through"), _T("throughout"), _T("thru"), _T("thus"), _T("to"),
         _T("together"), _T("too"), _T("toward"), _T("towards"), _T("under"), _T("until"), _T("up"),
         _T("upon"), _T("us"), _T("very"), _T("via"), _T("was"), _T("we"), _T("well"), _T("were"), _T("what"),
         _T("whatever"), _T("whatsoever"), _T("when"), _T("whence"), _T("whenever"),
         _T("whensoever"), _T("where"), _T("whereafter"), _T("whereas"), _T("whereat"),
         _T("whereby"), _T("wherefrom"), _T("wherein"), _T("whereinto"), _T("whereof"),
         _T("whereon"), _T("whereto"), _T("whereunto"), _T("whereupon"), _T("wherever"),
         _T("wherewith"), _T("whether"), _T("which"), _T("whichever"), _T("whichsoever"),
         _T("while"), _T("whilst"), _T("whither"), _T("who"), _T("whoever"), _T("whole"), _T("whom"),
         _T("whomever"), _T("whomsoever"), _T("whose"), _T("whosoever"), _T("why"), _T("will"),
         _T("with"), _T("within"), _T("without"), _T("would"), _T("xsubj"), _T("xcal"), _T("xauthor"),
         _T("xother "), _T("xnote"), _T("yet"), _T("you"), _T("your"), _T("yours"), _T("yourself"),
         _T("yourselves"),
         NULL
   };
}
   /*TODO:
   * extented english stop word set can be merged to main english stop words set
   * chinese thai, cjk and may be others that use english stop word list may be done as
   * const TCHAR* NonEnglishStopWords::THAI_STOP_WORDS = NonEnglishStopWords::ENGLISH_STOP_WORDS
   * so as to safely use CustomAnalyzer(L"thai>stop(th)")
   */
