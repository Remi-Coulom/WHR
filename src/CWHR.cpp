/////////////////////////////////////////////////////////////////////////////
//
// CWHR.cpp
//
// Rémi Coulom
//
// May, 2005
//
/////////////////////////////////////////////////////////////////////////////
#include "CWHR.h"
#include "CGameCollection.h"
#include "clktimer.h"
#include "chtime.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <xmmintrin.h>
#include <algorithm>

using namespace whr;

//
// Algorithm parameters
//
const float HessianEpsilon = 1.0f;
const int PrefetchBuffer = 10;

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CWHR::CWHR(const CGameCollection &gcInit):
 gc(gcInit),
 InitialLogLikelihood(0.0),
 LinkStrength(1000.0),
 InitialPriorWins(0.5)
{
 Reset();
}

/////////////////////////////////////////////////////////////////////////////
// Reset
/////////////////////////////////////////////////////////////////////////////
void CWHR::Reset()
{
 //
 // clear arrays
 //
 vpd.clear();
 vgd.clear();

 //
 // Initialize Handicap / Komi arrays
 //
 vHandicapGamma.resize(gc.GetMaxHandicap() + 1 +
                       gc.GetMaxKomi() - gc.GetMinKomi() + 1);
 KomiOffset = gc.GetMaxHandicap() + 1 - gc.GetMinKomi();

 vHandicapWins.resize(vHandicapGamma.size());
 vHandicapGames.resize(vHandicapGamma.size());
 vHandicapGradient.resize(vHandicapGamma.size());
 vHandicapHessian.resize(vHandicapGamma.size());

 std::fill(vHandicapWins.begin(), vHandicapWins.end(), 0);
 std::fill(vHandicapGames.begin(), vHandicapGames.end(), 0);
 std::fill(vHandicapGamma.begin(), vHandicapGamma.end(), 1.0f);

 //
 // Initialize vFirstIndex and vLastIndex
 //
 vFirstIndex.resize(gc.GetPlayers());
 vLastIndex.resize(gc.GetPlayers());
 std::fill(vFirstIndex.begin(), vFirstIndex.end(), -1);
 std::fill(vLastIndex.begin(), vLastIndex.end(), -1);
}

/////////////////////////////////////////////////////////////////////////////
// AddAllGames
/////////////////////////////////////////////////////////////////////////////
void CWHR::AddAllGames()
{
 for (int i = 0; i < gc.GetGames(); i++)
  AddGame(gc.GetGame(i));
}

/////////////////////////////////////////////////////////////////////////////
// AddGame
/////////////////////////////////////////////////////////////////////////////
void CWHR::AddGame(const CGame &game)
{
 int Day = game.GetDay();

 if (vgd.size())
  MaxDay = Day;
 else
 {
  MinDay = Day;
  MaxDay = Day;
 }

 CGameData gd;

 gd.WhitePD = FindOrCreatePlayerDay(game.GetWhite(), Day);
 gd.BlackPD = FindOrCreatePlayerDay(game.GetBlack(), Day);
 gd.NextWhiteGame = vpd[gd.WhitePD].FirstWhiteGame;
 gd.NextBlackGame = vpd[gd.BlackPD].FirstBlackGame;
 vpd[gd.WhitePD].FirstWhiteGame = int(vgd.size());
 vpd[gd.BlackPD].FirstBlackGame = int(vgd.size());
 vpd[gd.WhitePD].Games++;
 vpd[gd.BlackPD].Games++;

 vHandicapGames[game.GetHandicap()]++;
 vHandicapGames[game.GetKomi() + KomiOffset]++;

 gd.KomiIndex = game.GetKomi() + KomiOffset;
 gd.Handicap = game.GetHandicap();

 if (game.GetResult())
 {
  vpd[gd.BlackPD].Wins += 1.0f;
  vHandicapWins[gd.Handicap]++;
 }
 else
 {
  vpd[gd.WhitePD].Wins += 1.0f;
  vHandicapWins[gd.KomiIndex]++;
 }

 vgd.push_back(gd);
}

/////////////////////////////////////////////////////////////////////////////
// GetVirtualWins for pd
/////////////////////////////////////////////////////////////////////////////
float CWHR::GetVirtualWins(const CPlayerDay &pd,
                           const CPlayerDay &pdNext) const
{
#if 0
 float x = (LinkStrength * pdNext.Gamma * pd.Gamma) / (pdNext.Day - pd.Day);
 if (x < 50)
  x = 50;
 else if (x > 10000)
  x = 10000;
 return x;
#else
 return LinkStrength / float(pdNext.Day - pd.Day);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// FindOrCreatePlayerDay
/////////////////////////////////////////////////////////////////////////////
int CWHR::FindOrCreatePlayerDay(int Player, int Day)
{
 int i = vLastIndex[Player];

 if (i < 0 || Day > vpd[i].Day)
 {
  CPlayerDay pd;

  pd.Next = -1;
  pd.Previous = i;
  pd.Day = Day;
  pd.Player = Player;
  pd.FirstBlackGame = -1;
  pd.FirstWhiteGame = -1;
  pd.Gamma = 1.0;
  pd.Wins = 0.0;
  pd.Games = 0;

  if (i >= 0)
  {
   vpd[i].Next = int(vpd.size());
   pd.Gamma = vpd[i].Gamma;
  }
  else
   vFirstIndex[Player] = int(vpd.size());

  vpd.push_back(pd);

  return vLastIndex[Player] = int(vpd.size()) - 1;
 }
 else
  return i;
}

/////////////////////////////////////////////////////////////////////////////
// FindPlayerDay
/////////////////////////////////////////////////////////////////////////////
int CWHR::FindPlayerDay(int Player, int Day) const
{
 int i = vFirstIndex[Player];
 while (i >= 0)
 {
  if (vpd[i].Day == Day)
   break;
  i = vpd[i].Next;
 }
 return i;
}

/////////////////////////////////////////////////////////////////////////////
// ResetRatings
/////////////////////////////////////////////////////////////////////////////
void CWHR::ResetRatings()
{
 std::fill(vHandicapGamma.begin(), vHandicapGamma.end(), 1.0f);
 for (int i = int(vpd.size()); --i >= 0;)
  vpd[i].Gamma = 1.0;
 InitialLogLikelihood = GetLogLikelihood();
}

/////////////////////////////////////////////////////////////////////////////
// Compute the log-likelihood of the model
/////////////////////////////////////////////////////////////////////////////
double CWHR::GetLogLikelihood() const
{
 double Result = 0;

 //
 // Real games
 //
 for (int i = int(vgd.size()); --i >= 0;)
 {
  const CGame &game = gc.GetGame(i);
  int Black = vgd[i].BlackPD;
  int White = vgd[i].WhitePD;
  float hGamma = vHandicapGamma[game.GetHandicap()];
  float kGamma = vHandicapGamma[game.GetKomi() + KomiOffset];
  double BlackValue = vpd[Black].Gamma * hGamma;
  double WhiteValue = vpd[White].Gamma * kGamma;

  double p;
  if (game.GetResult())
   p = BlackValue / (BlackValue + WhiteValue);
  else
   p = WhiteValue / (BlackValue + WhiteValue);

  Result += std::log(p);
 }

 //
 // Temporal links between playerdays
 //
 for (int i = int(vpd.size()); --i >= 0;)
 {
  int Next = vpd[i].Next;
  if (Next >= 0)
  {
   double Wins = GetVirtualWins(vpd[i], vpd[Next]);
   double p = vpd[i].Gamma / (vpd[Next].Gamma + vpd[i].Gamma);
   Result += Wins * (std::log(p*(1 - p)));
  }
 }

 return Result;
}

/////////////////////////////////////////////////////////////////////////////
// Get link strength
/////////////////////////////////////////////////////////////////////////////
float CWHR::GetLinkStrength() const
{
 return LinkStrength;
}

/////////////////////////////////////////////////////////////////////////////
// Newton's method for komi and handicap
/////////////////////////////////////////////////////////////////////////////
void CWHR::NewtonKomiHandicap()
{
 //
 // Reset all gradients and Hessians
 //
 std::fill(vHandicapGradient.begin(), vHandicapGradient.end(), 0.0f);
 std::fill(vHandicapHessian.begin(), vHandicapHessian.end(), 0.0f);

 //
 // Loop over games
 //
 for (int i = int(vgd.size()); --i >= 0;)
 {
  const CGameData &gd = vgd[i];
  int Black = gd.BlackPD;
  int White = gd.WhitePD;

  float CKomi = vpd[White].Gamma;
  float DKomi = vpd[Black].Gamma * vHandicapGamma[gd.Handicap];
  float CHandicap = vpd[Black].Gamma;
  float DHandicap = vpd[White].Gamma * vHandicapGamma[gd.KomiIndex];

  float Div = 1.0f / (DKomi + DHandicap);

  vHandicapGradient[gd.Handicap] += CHandicap * Div;
  vHandicapHessian[gd.Handicap] += CHandicap * DHandicap * Div * Div;

  vHandicapGradient[gd.KomiIndex] += CKomi * Div;
  vHandicapHessian[gd.KomiIndex] += CKomi * DKomi * Div * Div;
 }

 //
 // Update gammas
 //
 for (int i = int(vHandicapGamma.size()); --i >= 0;)
  if (vHandicapGames[i] &&
      vHandicapWins[i] > 0 &&
      vHandicapWins[i] < vHandicapGames[i])
 {
  float G = float(vHandicapWins[i]) - vHandicapGamma[i] * vHandicapGradient[i];
  float H = -vHandicapGamma[i] * vHandicapHessian[i];
  H -= HessianEpsilon;
  vHandicapGamma[i] *= std::exp(-G/H);
 }
}

/////////////////////////////////////////////////////////////////////////////
// Prepare Fast Game Data
/////////////////////////////////////////////////////////////////////////////
void CWHR::PrepareFastGameData()
{
 //
 // Allocate vector
 //
 vfgd.resize(vgd.size() * 2 + 2 * PrefetchBuffer);
 for (int i = 2 * PrefetchBuffer; --i >= 0;)
  vfgd[vgd.size() * 2 + i].OpponentPlayerDay = 0;

 //
 // Loop over players
 //
 int FastIndex = 0;
 for (int Player = gc.GetPlayers(); --Player >= 0;)
 {
  //
  // Loop over playerdays of this player
  //
  for (int i = vFirstIndex[Player]; i >= 0; i = vpd[i].Next)
  {
   vpd[i].FirstFastGame = FastIndex;

   //
   // Games played as Black
   //
   for (int j = vpd[i].FirstBlackGame; j >= 0; j = vgd[j].NextBlackGame)
   {
    vfgd[FastIndex].MyAdvantage = vgd[j].Handicap;
    vfgd[FastIndex].OpponentPlayerDay = vgd[j].WhitePD;
    vfgd[FastIndex].OpponentAdvantage = vgd[j].KomiIndex;
    FastIndex++;
   }

   //
   // Games played as White
   //
   for (int j = vpd[i].FirstWhiteGame; j >= 0; j = vgd[j].NextWhiteGame)
   {
    vfgd[FastIndex].MyAdvantage = vgd[j].KomiIndex;
    vfgd[FastIndex].OpponentPlayerDay = vgd[j].BlackPD;
    vfgd[FastIndex].OpponentAdvantage = vgd[j].Handicap;
    FastIndex++;
   }
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Compute Hessian (and Gradient, and first pass of LU decomposition)
/////////////////////////////////////////////////////////////////////////////
void CWHR::ComputeHessian(const int Player, int fFast)
{
 //
 // Prior on the first playerday
 //
 int i = vFirstIndex[Player];
 {
  float div = 1.0f / (vpd[i].Gamma + 1.0f);
  vpd[i].G = InitialPriorWins * (1.0f - 2 * vpd[i].Gamma * div);
  vpd[i].h = - 2 * InitialPriorWins * vpd[i].Gamma * div * div;
  vpd[i].d = vpd[i].h;
 }

 //
 // Loop over playerdays of this player
 //
 while (1)
 {
  _mm_prefetch(&vpd[vpd[i].Next], _MM_HINT_T0);

  const float Gamma = vpd[i].Gamma;
  float H = 0;
  float G = 0;

  //
  // Loop over games of this day
  //
  if (fFast)
  {
   const int Limit = vpd[i].FirstFastGame + vpd[i].Games;
   for (int j = vpd[i].FirstFastGame; j < Limit; j++)
   {
    _mm_prefetch(&vpd[vfgd[j + PrefetchBuffer].OpponentPlayerDay], _MM_HINT_T0);
    _mm_prefetch(&vfgd[j + 2 * PrefetchBuffer], _MM_HINT_T0);
    float C = vHandicapGamma[vfgd[j].MyAdvantage];
    float D = vpd[vfgd[j].OpponentPlayerDay].Gamma *
              vHandicapGamma[vfgd[j].OpponentAdvantage];
    float inv = 1.0f / (C * Gamma + D);
    G += C * inv;
    H += (C * D) * (inv * inv);
   }
  }
  else
  {
   //
   // Games played as Black
   //
   for (int j = vpd[i].FirstBlackGame; j >= 0; j = vgd[j].NextBlackGame)
   {
    float C = vHandicapGamma[vgd[j].Handicap];
    float D = vpd[vgd[j].WhitePD].Gamma *
              vHandicapGamma[vgd[j].KomiIndex];
    float inv = 1.0f / (C * Gamma + D);
    G += C * inv;
    H += (C * D) * (inv * inv);
   }

   //
   // Games played as White
   //
   for (int j = vpd[i].FirstWhiteGame; j >= 0; j = vgd[j].NextWhiteGame)
   {
    float C = vHandicapGamma[vgd[j].KomiIndex];
    float D = vpd[vgd[j].BlackPD].Gamma *
              vHandicapGamma[vgd[j].Handicap];
    float inv = 1.0f / (C * Gamma + D);
    G += C * inv;
    H += (C * D) * (inv * inv);
   }
  }

  //
  // Store H and G
  //
  vpd[i].d -= H * Gamma + HessianEpsilon;
  vpd[i].h -= H * Gamma + HessianEpsilon;
  vpd[i].G += vpd[i].Wins - Gamma * G;

  //
  // Link to next playerday
  //
  int Next = vpd[i].Next;
  if (Next >= 0)
  {
#if 0
   float NextGamma = vpd[Next].Gamma;
   float Sum = Gamma + NextGamma;
   float VirtualWins = GetVirtualWins(vpd[i], vpd[Next]);
   float VirtualLosses = VirtualWins;
   float g = (VirtualWins + VirtualLosses) / Sum;
   float b = ((VirtualWins + VirtualLosses) * Gamma * NextGamma) / (Sum * Sum);
   vpd[i].b = b;
   vpd[i].d -= b;
   vpd[i].h -= b;
   vpd[i].G += VirtualWins - Gamma * g;

   float a = b / vpd[i].d;

   vpd[Next].d = -b - a * b;
   vpd[Next].h = -b;
   vpd[Next].G = VirtualLosses - NextGamma * g - a * vpd[i].G;
#else
   float NextGamma = vpd[Next].Gamma;
   float VirtualWins = GetVirtualWins(vpd[i], vpd[Next]);
   float b = VirtualWins * 0.5f;
//   float G = std::log(NextGamma / Gamma) * VirtualWins * 0.5;
   float G = ((NextGamma - Gamma) / Gamma) * VirtualWins * 0.5f;
   vpd[i].b = b;
   vpd[i].d -= b;
   vpd[i].h -= b;
   vpd[i].G += G;

   float a = b / vpd[i].d; // terms in - a * are the LU decomposition

   vpd[Next].d = -b - a * b;
   vpd[Next].h = -b;
   vpd[Next].G = -G - a * vpd[i].G;
#endif

   i = Next;
  }
  else
   break;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Fast approximation of std::exp
/////////////////////////////////////////////////////////////////////////////
static inline float MyExp(float x)
{
 float x2 = x * x;
 if (x2 < 0.25f)
  return 1.0f + x + x2 * 0.5f;
 else
  return std::exp(x);
}

/////////////////////////////////////////////////////////////////////////////
// Apply Newton's method to update all the player-days of one player
/////////////////////////////////////////////////////////////////////////////
void CWHR::Newton(const int Player)
{
 int i = vLastIndex[Player];

 vpd[i].G /= vpd[i].d;
 vpd[i].Gamma *= MyExp(-vpd[i].G);

 int Next;
 i = vpd[Next = i].Previous;

 while (i >= 0)
 {
  vpd[i].G = (vpd[i].G - vpd[i].b * vpd[Next].G) / vpd[i].d;
  vpd[i].Gamma *= MyExp(-vpd[i].G);

  i = vpd[Next = i].Previous;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Compute variance, and store it in G
/////////////////////////////////////////////////////////////////////////////
void CWHR::ComputeVariance(const int Player)
{
 //
 // Compute Hessian and LU-decompose it
 //
 ComputeHessian(Player);

 //
 // UL-decomposition
 //
 int i = vLastIndex[Player];

 vpd[i].dprime = vpd[i].h;
 vpd[i].G = -1 / vpd[i].d;

 int Next;
 i = vpd[Next = i].Previous;

 while (i >= 0)
 {
  vpd[i].dprime = vpd[i].h - vpd[i].b * vpd[i].b / vpd[Next].dprime;
  vpd[i].G = vpd[Next].dprime /
             (vpd[i].b * vpd[i].b - vpd[i].d * vpd[Next].dprime);

  i = vpd[Next = i].Previous;
 }
}

/////////////////////////////////////////////////////////////////////////////
// One iteration over all players
/////////////////////////////////////////////////////////////////////////////
void CWHR::OneIterationOverPlayers(int fFast)
{
 for (int i = gc.GetPlayers(); --i >= 0;)
  if (vFirstIndex[i] >= 0)
  {
   ComputeHessian(i, fFast);
   Newton(i);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Iterations of the Newton algorithm
/////////////////////////////////////////////////////////////////////////////
void CWHR::NewtonIterate(std::ostream &os, int Iterations)
{
 PrepareFastGameData();
 ResetRatings();

 int Counter = 0;

 flag.Reset();
 flag.Activate();

 CClockTimer clktimer;
 CTime timeTotal = 0;

 while(!flag.IsSet() && Counter < Iterations)
 {
  OneIterationOverPlayers(1);
  NewtonKomiHandicap();
  Counter++;
  if (Counter % 10 == 0)
  {
   timeTotal += clktimer.GetInterval();
   os << std::setw(10) << float(timeTotal)/100 << ' ';
   os << std::setw(5) << Counter << ' ';
   os << GetRelativeLL() << ' ';
   os << '\n';
   os.flush();
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Predict ???: should integrate, with uncertainty
/////////////////////////////////////////////////////////////////////////////
float CWHR::Predict(int Handicap,
                    int Komi,
                    int Black,
                    int White,
                    int Day)
{
 //
 // Return 0.5 if no game
 //
 if (vLastIndex[Black] < 0 && vLastIndex[White] < 0)
  return 0.5;

 //
 // Find rating and uncertainty of the black player
 //
 int BlackPD = FindOrCreatePlayerDay(Black, Day);
 ComputeHessian(Black, 0);
 Newton(Black);
 float BlackGamma = vpd[BlackPD].Gamma;
 float BlackSigma2 = -1 / vpd[vLastIndex[Black]].d;

 //
 // Find rating and uncertainty of the white player
 //
 int WhitePD = FindOrCreatePlayerDay(White, Day);
 ComputeHessian(White, 0);
 Newton(White);
 float WhiteGamma = vpd[WhitePD].Gamma;
 float WhiteSigma2 = -1 / vpd[vLastIndex[White]].d;

 //
 // Handicap and komi (???: and their uncertainties ???)
 //
 float KomiGamma = vHandicapGamma[Komi + KomiOffset];
 float HandicapGamma = vHandicapGamma[Handicap];

 //
 // Mean and standard deviation of the strength difference
 //
 float DeltaR = std::log((BlackGamma * HandicapGamma) /
                         (WhiteGamma * KomiGamma));
 float Sigma = std::sqrt(BlackSigma2 + WhiteSigma2);

 //
 // Compute prediction by integrating over uncertainty
 //
#if 1
 const int Steps = 0;
#else
 const int Steps = 4;
#endif
 const float Step = 0.5f; // multiple of Sigma
 float TotalWeight = 0.0f;
 float Integral = 0.0f;

 for (int i = -Steps; i <= Steps; i++)
 {
  float x = Step * float(i);
  float p = std::exp(-x * x / 2.0f);
  float d = DeltaR + Sigma * x;
  float f = 1.0f / (1.0f + std::exp(-d));

  TotalWeight += p;
  Integral += p * f;
 }

 Integral /= TotalWeight;

 //
 // Debug output
 //
#if 0
 float B = BlackGamma * HandicapGamma;
 float W = WhiteGamma * KomiGamma;

 std::cout << std::setw(10) << DeltaR << ' '
           << std::setw(10) << Sigma << ' '
           << std::setw(10) << B / (B + W) << ' '
           << std::setw(10) << Integral << '\n';
#endif

 return Integral;
}

/////////////////////////////////////////////////////////////////////////////
// ComputeDrift
/////////////////////////////////////////////////////////////////////////////
void CWHR::ComputeDrift()
{
 const int Size = MaxDay - MinDay + 1;

 vGameCount.resize(Size + 2 * KernelRadius);
 vTotalElo.resize(Size + 2 * KernelRadius);
 vFilteredCount.resize(Size);
 vFilteredElo.resize(Size);

 //
 // Reset totals to zero
 //
 for (int i = Size + 2 * KernelRadius; --i >= 0;)
 {
  vGameCount[i] = 0;
  vTotalElo[i] = 0.0;
 }

 //
 // Count games and average player strength
 //
 for (int i = int(vgd.size()); --i >= 0;)
 {
  const CGame &game = gc.GetGame(i);
  int j = game.GetDay() - MinDay + KernelRadius;

  vGameCount[j]++;
  vTotalElo[j] += GammaToElo(vpd[vgd[i].BlackPD].Gamma) +
                  GammaToElo(vpd[vgd[i].WhitePD].Gamma);
 }

 //
 // Generate convolution kernel
 //
 std::vector<float> vKernel(KernelRadius);
 {
  const float Sigma = KernelRadius * 0.5f;
  float Total = 1.0f;
  for (int i = KernelRadius; --i > 0;)
  {
   float x = std::exp(-float(i * i) / (2.0f * Sigma * Sigma));
   vKernel[i] = x;
   Total += 2.0f * x;
  }

  float Normalization = 1.0f / Total;
  vKernel[0] = Normalization * 0.5f;
  for (int i = KernelRadius; --i > 0;)
   vKernel[i] *= Normalization;
 }

 //
 // Filter with convolution kernel
 //
 for (int i = Size; --i >= 0;)
 {
  vFilteredCount[i] = 0.0;
  vFilteredElo[i] = 0.0;

  for (int k = KernelRadius; --k >= 0;)
  {
   int j = i + KernelRadius;
   vFilteredElo[i] += (vTotalElo[j + k] + vTotalElo[j - k]) * vKernel[k];
   vFilteredCount[i] += float(vGameCount[j + k] + vGameCount[j - k]) * vKernel[k];
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// RemoveDrift
/////////////////////////////////////////////////////////////////////////////
void CWHR::RemoveDrift()
{
 ComputeDrift();
 std::vector<float> vGammaCorrection(MaxDay - MinDay + 1);

 //
 // First pass: compute gamma correction for every day
 //
 for (int i = MaxDay - MinDay + 1; --i >= 0;)
 {
  float DriftElo = vFilteredElo[i] / (2.0f * vFilteredCount[i]);
  float Correction = 1.0f / EloToGamma(DriftElo);
  if (std::isnan(Correction) || std::isinf(Correction))
   vGammaCorrection[i] = 1.0f;
  else
   vGammaCorrection[i] = Correction;
 }

 //
 // Second pass: apply gamma correction to all playerdays
 //
 for (int i = int(vpd.size()); --i >= 0;)
  vpd[i].Gamma *= vGammaCorrection[vpd[i].Day - MinDay];
}

/////////////////////////////////////////////////////////////////////////////
// Extract
/////////////////////////////////////////////////////////////////////////////
float CWHR::Extract(int p, int q, CGameCollection &gcSlice) const
{
 std::vector<float> vTotalStrength(gc.GetGames());

 //
 // Compute total strength for all games, and sort
 //
 for (int i = gc.GetGames(); --i >= 0;)
  vTotalStrength[i] = vpd[vgd[i].BlackPD].Gamma * vpd[vgd[i].WhitePD].Gamma;
 std::sort(vTotalStrength.begin(), vTotalStrength.end());

 //
 // Determine range of total strength for this slice
 //
 int Size = int(vTotalStrength.size());
 int MinIndex = ((p - 1) * Size) / q;
 int MaxIndex = (p * Size) / q - 1;
 float min = vTotalStrength[MinIndex];
 float max = vTotalStrength[MaxIndex];

 //
 // Build gcSlice
 //
 gcSlice.Reset(gc);
 float TotalLog = 0;
 int Games = 0;
 for (int i = 0; i < gc.GetGames(); i++)
 {
  float TotalStrength = vpd[vgd[i].BlackPD].Gamma * vpd[vgd[i].WhitePD].Gamma;
  if (TotalStrength >= min && TotalStrength <= max)
  {
   gcSlice.AddGame(gc.GetGame(i));
   TotalLog += std::log(TotalStrength);
   Games++;
  }
 }

 return 0.5f * TotalLog / float(Games);
}
