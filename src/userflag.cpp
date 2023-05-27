////////////////////////////////////////////////////////////////////////////
//
// userflag.cpp
//
// CUserFlag class
//
// Remi Coulom
//
// September, 1996
//
////////////////////////////////////////////////////////////////////////////
#include <csignal>
#ifndef SIGINT
#define SIGINT 2
#endif

#include "debug.h"
#include "userflag.h"

#include <algorithm>

std::vector<int> CUserFlag::vState;

////////////////////////////////////////////////////////////////////////////
// Signal handler for user interrupt
////////////////////////////////////////////////////////////////////////////
void CDECL CUserFlag::InterruptSignalHandler(int sig)
{
 signal(sig, (void (CDECL *)(int))SIG_IGN);
 std::fill(vState.begin(), vState.end(), 1);
 signal(sig, InterruptSignalHandler);
}

////////////////////////////////////////////////////////////////////////////
// Default constructor
////////////////////////////////////////////////////////////////////////////
CUserFlag::CUserFlag():
 Index(vState.size())
{
 vState.push_back(0);
 Activate();
}

////////////////////////////////////////////////////////////////////////////
// Function to ignore SIGINT signals
////////////////////////////////////////////////////////////////////////////
void CUserFlag::DeActivate()
{
 signal(SIGINT, (void (CDECL *)(int))SIG_IGN);
}

////////////////////////////////////////////////////////////////////////////
// Function to reset SIGINT signals
////////////////////////////////////////////////////////////////////////////
void CUserFlag::Activate()
{
 signal(SIGINT, InterruptSignalHandler);
}

////////////////////////////////////////////////////////////////////////////
// Function to test the flag
////////////////////////////////////////////////////////////////////////////
int CUserFlag::IsSet() const
{
 return vState[Index];
}

////////////////////////////////////////////////////////////////////////////
// Function to reset the flag
////////////////////////////////////////////////////////////////////////////
void CUserFlag::Reset()
{
 vState[Index] = 0;
}

////////////////////////////////////////////////////////////////////////////
// Function to set the flag
////////////////////////////////////////////////////////////////////////////
void CUserFlag::Set()
{
 vState[Index] = 1;
}