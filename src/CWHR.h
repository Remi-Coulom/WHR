/////////////////////////////////////////////////////////////////////////////
//
// CWHR.h
//
// Rémi Coulom
//
// May, 2005
//
/////////////////////////////////////////////////////////////////////////////
#ifndef WHR_CWHR_Declared
#define WHR_CWHR_Declared

#include <vector>
#include <iosfwd>
#include <cmath>

#include "userflag.h"
#include "CGameCollection.h"

namespace whr {

 class CPlayerDay { // pd ///////////////////////////////////////////////////
  public:
  int Player;
  int Day;
  int Games;
  int FirstFastGame;

  int Next;
  int Previous;
  int FirstBlackGame;
  int FirstWhiteGame;

  float Dummy;
  float Gamma;
  float Wins;
  float G;

  float d;
  float b;
  float dprime;
  float h;
 } __attribute__ ((aligned (64)));

 struct CGameData // gd /////////////////////////////////////////////////////
 {
  int BlackPD; // PlayerDay of Black
  int NextWhiteGame; // Next white game played in the same day
  int KomiIndex;
  int Handicap;
  int WhitePD; // PlayerDay of White
  int NextBlackGame; // Next black game played in the same day
 };

 struct CFastGameData // fgd
 {
  int MyAdvantage;
  int OpponentPlayerDay;
  int OpponentAdvantage;
 };

 class CWHR // whr /////////////////////////////////////////////////////////
 {
  public: //////////////////////////////////////////////////////////////////
   CUserFlag flag;

   const CGameCollection &gc;
   double InitialLogLikelihood;

   //
   // Game data
   //
   std::vector<CGameData> vgd;
   std::vector<CFastGameData> vfgd;

   //
   // PlayerDays
   //
   std::vector<CPlayerDay> vpd;
   std::vector<int> vFirstIndex; // First PlayerDay of Player
   std::vector<int> vLastIndex; // Last PlayerDay of Player

   //
   // Days
   //
   int MaxDay;
   int MinDay;

   //
   // Parameters of the Bradley-Terry model
   //
   std::vector<float> vHandicapGamma;
   int KomiOffset;

   //
   // Handicap and Komi data
   //
   std::vector<int> vHandicapGames;
   std::vector<int> vHandicapWins;
   std::vector<float> vHandicapGradient;
   std::vector<float> vHandicapHessian;

   //
   // Prior
   //
   float LinkStrength;
   float InitialPriorWins;
   float GetVirtualWins(const CPlayerDay &pd, const CPlayerDay &pdNext) const;

   //
   // Drift data
   //
   enum {KernelRadius = 100};
   std::vector<int> vGameCount;
   std::vector<float> vTotalElo;
   std::vector<float> vFilteredCount;
   std::vector<float> vFilteredElo;

   //
   // Funtion to fill some arrays during construction
   //
   int FindOrCreatePlayerDay(int Player, int Day);
   int FindPlayerDay(int Player, int Day) const;
   void PrepareFastGameData();

  public: ///////////////////////////////////////////////////////////////////
   CWHR(const CGameCollection &gcInit);

   //
   // Basic functions
   //
   void ResetRatings();
   double GetLogLikelihood() const;
   double GetRelativeLL() const
    {return GetLogLikelihood() - InitialLogLikelihood;}
   int GetPlayerDays() const {return int(vpd.size());}
   int GetGames() const {return int(vgd.size());}
   int Lookup(const std::string &sName) {return gc.Lookup(sName);}
   float GetLinkStrength() const;

   //
   // Second-order information for Newton and confidence intervals
   //
   void ComputeHessian(const int Player, int fFast = 0);
   void Newton(const int Player);
   void NewtonKomiHandicap();
   void ComputeVariance(const int Player);
   void OneIterationOverPlayers(int fFast);
   void NewtonIterate(std::ostream &os, int Iterations);

   //
   // CIncrementalPredictor stuff
   //
   void Reset();
   void AddAllGames();
   void AddGame(const CGame &game);
   float Predict(int Handicap,
                 int Komi,
                 int Black,
                 int White,
                 int Day);


   //
   // Drift stuff
   //
   void ComputeDrift();
   void RemoveDrift();

   //
   // Functions to dump various data to files
   //
   void PrintPlayerDays(std::ostream &os) const;
   void PrintVictories(std::ostream &os) const;
   void PrintRatingHistory(std::ostream &os, int Player) const;
   void PrintRatingConfidence(std::ostream &os, int Player);
   void PrintHandicapRatings(std::ostream &os) const;
   void PrintRatingCloud(std::ostream &os) const;
   void PrintDriftStats(std::ostream &os);
   void PrintWinningFrequency(std::ostream &os, int N);

   //
   // Read/Write daily ratings of players
   //
   void WriteRatings(std::ostream &os) const;
   void ReadRatings(std::istream &is);

   //
   // Convert gammas to elos
   //
   static float GammaToElo(float Gamma) {return 400.0f * std::log10(Gamma);}
   static float EloToGamma(float Elo) {return std::pow(10.0f, Elo / 400.0f);}

   //
   // Prior analysis
   //
   void AnalyzePrior(float minElo,
                     float maxElo,
                     int minDay,
                     int maxDay,
                     int Horizon,
                     float &Average,
                     float &Variance,
                     int &N);

   //
   // Extract a subset of games (p-th part out of q, p in 1...q)
   //
   float Extract(int p, int q, CGameCollection &gcSlice) const;
 };
}

#endif
