/////////////////////////////////////////////////////////////////////////////
//
// CGame.h
//
// May, 2005
//
/////////////////////////////////////////////////////////////////////////////
#ifndef WHR_CGame_Declared
#define WHR_CGame_Declared

namespace whr {
 class CGame // game
 {
  private: ///////////////////////////////////////////////////////////////////
   int Day;
   int Handicap;
   int Komi; // times 2
   int Result; // 0: White wins, 1: Black wins ??? Draw ???
   int Black;
   int White;
 
  public: ////////////////////////////////////////////////////////////////////
   int GetDay() const {return Day;}
   int GetHandicap() const {return Handicap;}
   int GetKomi() const {return Komi;}
   int GetResult() const {return Result;}
   int GetBlack() const {return Black;}
   int GetWhite() const {return White;}
 
   void SetDay(int NewDay) {Day = NewDay;}
   void SetHandicap(int NewHandicap) {Handicap = NewHandicap;}
   void SetKomi(int NewKomi) {Komi = NewKomi;}
   void SetResult(int NewResult) {Result = NewResult;}
   void SetWhite(int NewWhite) {White = NewWhite;}
   void SetBlack(int NewBlack) {Black = NewBlack;}
 };
}

#endif
