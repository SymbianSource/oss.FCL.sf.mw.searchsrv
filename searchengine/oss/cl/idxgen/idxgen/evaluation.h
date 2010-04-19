/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
//#include "malloc.h"

using namespace std; 

#ifndef QUERY_CHAR 
#define QUERY_CHAR char
#define Q(x) x
#endif



/*const QUERY_CHAR* sample[] = {
	// From all's well that ends well
    Q("'T were all one that I should love a bright particular star, and think to wed it."),
    Q("The hind, that would be mated by the lion, Must die for love. "),
	Q("Our remedies oft in ourselves do lie, Which we ascribe to Heaven. "),
	Q("Service is no heritage."), 
    Q("Even so it was with me, when I was young: If we are nature’s, these are ours; this thorn Doth to our rose of youth rightly belong: Our blood to us, this to our blood is born; It is the show and seal of nature’s truth, Where love’s strong passion is impress’d in youth: By our remembrances of days foregone, Such were our faults; — or then we thought them none. Her eye is sick on’t: I observe her now. "),
	// Anthony and cleopatra
	Q("Nay, but this dotage of our general's O'erflows the measure."),
	Q("There's beggary in the love that can be reckon'd."),
	Q("In nature's infinite book of secrecy A little I can read."),
	Q("Eternity was in our lips and eyes. "),
	Q("My salad days, When I was green in judgment, cold in blood, To say as I said then!"),
	// As you like it
	Q("Let us sit and mock the good housewife Fortune from her wheel, that her gifts may henceforth be bestowed equally."),
	Q("I would we could do so; for her benefits are mightily misplaced: and the bountiful blind woman doth most mistake in her gifts to women."),
	Q("Tis true; for those that she makes fair, she scarce makes honest; and those that she makes honest, she makes very ill-favouredly."),
	Q("Nay; now thou goest from Fortune's office to Nature's: Fortune reigns in gifts of the world, not in the lineaments of Nature."),
	Q("No? When Nature hath made a fair creature, may she not by Fortune fall into the fire? — Though Nature hath given us wit to flout at Fortune, hath not Fortune sent in this fool to cut off the argument? "),
	// The comedy of errors
	Q("The pleasing punishment that women bear."),
	Q("A wretched soul, bruis'd with adversity, We bid be quiet, when we hear it cry; But, were we burden'd with like weight of pain, As much or more we should ourselves complain. "),
	Q("Every why hath a wherefore. "),
	Q("A hungry lean-fac'd villain, A mere anatomy. "),
	// Corolanius
	Q("Faith, there have been many great men that have flatter'd the people, who ne'er loved them. "),
	Q("Hear you this Triton of the minnows?"),
	Q("His nature is too noble for the world: He would not flatter Neptune for his trident, Or Jove for’s power to thunder."),
	Q("O world, thy slippery turns! Friends now fast sworn, Whose double bosoms seems to wear one heart, Whose hours, whose bed, whose meal and exercise Are still together, who twin, as 'twere, in love Unseparable, shall within this hour, On a dissension of a doit, break out To bitterest enmity; so fellest foes, Whose passions and whose plots have broke their sleep To take the one the other, by some chance, Some trick not worth an egg, shall grow dear friends And interjoin their issues. So with me:— My birthplace hate I, and my love's upon This enemy town.—I'll enter; if he slay me, He does fair justice; if he give me way, I'll do his country service."),
	Q("Let me have war, say I; it exceeds peace as far as day does night; it's spritely, waking, audible, and full of vent. Peace is a very apoplexy, lethargy; mull'd, deaf, sleepy, insensible; a getter of more bastard children than war's a destroyer of men."),
	Q("I'll never Be such a gosling to obey instinct, but stand As if a man were author of himself And knew no other kin. "),
	// Cymberline
	Q("His fortunes all lie speechless, and his name is at last gasp. "),
	Q("Boldness be my friend! Arm me, audacity, from head to foot!"),
	Q("Every Jack-slave hath his belly-ful of fighting. "),
	Q("The crickets sing, and man's o'er-labour'd sense Repairs itself by rest. "),
	Q("'Tis her breathing that Perfumes the chamber thus. "),
	// Hamlet
	Q("Not so my lord; I am too much i' the sun. "), 
	Q("Seems, madam! Nay, it is; I know not seems."),
	Q("O, that this too too solid flesh would melt, Thaw, and resolve itself into a dew."),
	Q("Thrift, thrift, Horatio! The funeral bak'd meats Did coldly furnish forth the marriage tables. "),
	Q("Be thou familiar, but by no means vulgar. "),
	Q("Give every man thy ear, but few thy voice; Take each man's censure, but reserve thy judgment. "),
	Q("To be, or not to be, — that is the question"),
	NULL
};*/

MEMORY_TEST(
class MemoryInfo {
	public: 

		int malloc() {
			return mallinfo().uordblks;
		}

		int total() {
			return mallinfo().uordblks + mallinfo().hblkhd;
		}

		void record() {
			malloc_peak = max(malloc_peak, malloc()); 
			malloc_aver = (updates / (updates+1.)) * malloc_aver + (1./(updates+1.)) * malloc(); 
			total_peak = max(total_peak, total()); 
			total_aver = (updates / (updates+1.)) * total_aver + (1./(updates+1.)) * total();

			updates++; 
		}
		
		void printCurrent(char* at) {
			cout<<"Malloc usage "<<malloc()<<" at "<<at<<endl;
			cout<<"Total usage "<<total()<<" at "<<at<<endl;
		}
		
		void print() {
			cout<<"Malloc peak: "<<malloc_peak<<" bytes"<<endl; 
			cout<<"Malloc average: "<<((int)malloc_aver)<<" bytes"<<endl; 
			cout<<"Total peak: "<<total_peak<<" bytes"<<endl; 
			cout<<"Total average: "<<((int)total_aver)<<" bytes"<<endl; 
		}
	public:
	 
		double malloc_aver;
		int malloc_peak;
		double total_aver;
		int total_peak;
		double updates;  
		
};
)
	
MEMORY_TEST(MemoryInfo memory);


