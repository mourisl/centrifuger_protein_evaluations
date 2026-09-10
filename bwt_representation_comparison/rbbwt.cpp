#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <string>
#include <fstream>
#include <iostream>
#include <chrono>

#include "SequenceCompactor.hpp"
#include "Sequence_WaveletTree.hpp"
#include "Sequence_RunLength.hpp"
#include "Sequence_Hybrid.hpp"
#include "Sequence_RunBlock.hpp"
#include "Sequence_RunBlockOneTree.hpp"
#include "FMBuilder.hpp"
#include "FMIndex.hpp"

// Usage: ./a.out (sequence_file|bwt_idx|plain_bwt) [options: -b XXX (0: sequence file need to build BWT. 1: bwt index with alphabet. 2: BWT sequence, no need to build from sequence); -a XXX [ACGT]: alphabet set; -m XXX [24G]: memory; -c XXX ['0']: test character; --dump: dump plain BWT (works for sequence_file input)]
using namespace std::chrono ;
using timer = std::chrono::high_resolution_clock;

using namespace compactds ;

int main(int argc, char *argv[])
{
  int i ;
  std::string seq ;
  FixedSizeElemArray s ;
  
  //char abList[128] = "ACGT" ; //"ACDEFGHIKLMNPQRSTVWY" for aa
  char memory[128] = "24G" ;
  FixedSizeElemArray BWT ;
  size_t n = 0 ;
  const size_t maxTestCnt = 10000000 ;
  int threadCnt = 4 ;
  int bwtStatus = 0 ;
  char *abList = NULL ;
  char testChr = '0' ;
	bool dumpPlainBwt = false ;
  
  for (i = 2 ; i < argc ; ++i)
  {
    if (!strcmp(argv[i], "-b"))
    {
      bwtStatus = atoi(argv[i + 1]) ;
    }
    else if (!strcmp(argv[i], "-t"))
    {
      threadCnt = atoi(argv[i+1]) ;
      ++i ;
    }
    else if (!strcmp(argv[i], "-a"))
    {
      abList = strdup(argv[i+1]) ;
      ++i ;
    }
    else if (!strcmp(argv[i], "-m"))
    {
      strcpy(memory, argv[i+1]) ;
      ++i ;
    }
    else if (!strcmp(argv[i], "-c"))
    {
      testChr = argv[i + 1][0] ;
      ++i ;
    }
		else if (!strcmp(argv[i], "--dump"))
		{
			dumpPlainBwt = true ;
		}
  }

  if (bwtStatus == 0)
  {
    std::ifstream ifs(argv[1], std::ifstream::in) ;
    std::getline(ifs, seq) ;
    
    Alphabet alphabet ;
		if (abList == NULL)
		{
			alphabet.InitFromSequence(seq.c_str(), seq.length()) ;
		  alphabet.GetAlphabetList(&abList, true) ;
		}
		else
		{
			alphabet.InitFromList(abList, strlen(abList)) ;
		}

    SequenceCompactor seqCompactor ; 
    seqCompactor.Init(abList, s, seq.length()) ;
    seqCompactor.Compact(seq.c_str(), s) ;

    n = s.GetSize() ;
    struct _FMBuilderParam param ;
    struct _FMIndexAuxData fmAuxData ;
    param.threadCnt = threadCnt ;
    param.saBlockSize = n / param.threadCnt ;
    param.precomputeWidth = 2 ; 
    FMBuilder::InferParametersGivenMemory(n, strlen(abList), Utils::SpaceStringToBytes(memory), param) ;
    size_t firstISA = 0 ;
    FMBuilder::Build(s, n, strlen(abList),
        BWT, firstISA, param) ;
    param.Free() ;
    char filename[512] ;
    sprintf(filename, "%s.idx", argv[1]) ;
    FILE *fp = fopen(filename, "w") ;
    alphabet.Save(fp) ;
    BWT.Save(fp) ;
    fclose(fp) ;

		if (dumpPlainBwt)
		{
			sprintf(filename, "%s.bwt", argv[1]) ;
			FILE *fp = fopen(filename, "w") ;
			for (i = 0 ; i < n ; ++i)
			{
				fprintf(fp, "%c", abList[BWT.Read(i)]) ;
			}
			fclose(fp) ;
		}
  }
  else if (bwtStatus == 1)
  {
    Alphabet alphabet ;

    FILE *fp = fopen(argv[1], "r") ;
    alphabet.Load(fp) ;
    BWT.Load(fp) ;
    fclose(fp) ;
    
    alphabet.GetAlphabetList(&abList, true) ;
    n = BWT.GetSize() ;

    //BWT.Print(stdout, '\n' ) ;
  }
  else if (bwtStatus == 2)
  {
    std::ifstream ifs(argv[1], std::ifstream::in) ;
    std::getline(ifs, seq) ;
    
    Alphabet alphabet ;
    alphabet.InitFromSequence(seq.c_str(), seq.length()) ;
    alphabet.GetAlphabetList(&abList, true) ;

    SequenceCompactor seqCompactor ; 
    seqCompactor.Init(abList, BWT, seq.length()) ;
    seqCompactor.Compact(seq.c_str(), BWT) ;

    n = BWT.GetSize() ;
  }

  printf("Total size: %lu %s\n", n, abList) ;

  if (1)
  {
    Sequence_WaveletTree<> plbwt ; // plain bwt
    plbwt.SetSelectSpeed(0) ;
    plbwt.Init(BWT, n, abList) ;
    printf("Plain bwt space (bytes): %lu\n", plbwt.GetSpace()) ;
  
    auto start = timer::now();
    size_t check = 0 ;
    size_t i ;
    for (i = 0 ; i < n && i < maxTestCnt ; ++i)
    {
      size_t x = plbwt.Rank(testChr, i) ;
      check += x ;
    }
    auto stop = timer::now();
    std::cout << "# rank time (ns) from " << i << "  = " << duration_cast<nanoseconds>(stop-start).count()/(double)i << std::endl;
    std::cout << "# rank sum = " << check << std::endl;
  }
  
  if (1)
  {
    Sequence_RunLength rlbwt ;
    rlbwt.Init(BWT, n, abList) ;
    rlbwt.PrintStats() ;
    printf("Runlength bwt space (bytes): %lu\n", rlbwt.GetSpace()) ;
    
    auto start = timer::now();
    size_t check = 0 ;
    size_t i ;
    for (i = 0 ; i < n && i < maxTestCnt ; ++i)
    {
      size_t x = rlbwt.Rank(testChr, i) ;
      check += x ;
    }
    auto stop = timer::now();
    std::cout << "# rank time (ns) from " << i << "  = " << duration_cast<nanoseconds>(stop-start).count()/(double)i << std::endl;
    std::cout << "# rank sum = " << check << std::endl;
  }
  
  if (0) 
  {
    Sequence_Hybrid hybbwt ;
    //hybbwt.SetBlockSize(8) ;
    hybbwt.Init(BWT, n, abList) ;
    hybbwt.PrintStats() ;
    printf("Hybrid bwt space (bytes): %lu\n", hybbwt.GetSpace()) ;

    auto start = timer::now();
    size_t check = 0 ;
    size_t i ;
    for (i = 0 ; i < n && i < maxTestCnt ; ++i)
    {
      size_t x = hybbwt.Rank(testChr, i) ;
      check += x ;
    }
    auto stop = timer::now();
    std::cout << "# rank time (ns) from " << i << "  = " << duration_cast<nanoseconds>(stop-start).count()/(double)i << std::endl;
    std::cout << "# rank sum = " << check << std::endl;
  }
  
  {
    Sequence_RunBlock rbbwt ;
    //rbbwt.SetBlockSize(4) ;
    rbbwt.Init(BWT, n, abList) ;
    rbbwt.PrintStats() ;
    printf("RunBlock bwt space (bytes): %lu\n", rbbwt.GetSpace()) ;

    auto start = timer::now();
    size_t check = 0 ;
    size_t i ;
    for (i = 0 ; i < n && i < maxTestCnt ; ++i)
    {
      size_t x = rbbwt.Rank(testChr, i) ;
      check += x ;
    }
    auto stop = timer::now();
    std::cout << "# rank time (ns) from " << i << "  = " << duration_cast<nanoseconds>(stop-start).count()/(double)i << std::endl;
    std::cout << "# rank sum = " << check << std::endl;

    /*FILE *fp = fopen("rbbwt.idx", "w") ;
    rbbwt.Save(fp) ;
    fclose(fp) ;*/
  }
  
  {
    Sequence_RunBlockOneTree rbbwt ;
    //rbbwt.SetBlockSize(4) ;
    rbbwt.Init(BWT, n, abList) ;
    rbbwt.PrintStats() ;
    printf("RunBlockOneTree bwt space (bytes): %lu\n", rbbwt.GetSpace()) ;

    auto start = timer::now();
    size_t check = 0 ;
    size_t i ;
    for (i = 0 ; i < n && i < maxTestCnt ; ++i)
    {
      size_t x = rbbwt.Rank(testChr, i) ;
      check += x ;
    }
    auto stop = timer::now();
    std::cout << "# rank time (ns) from " << i << "  = " << duration_cast<nanoseconds>(stop-start).count()/(double)i << std::endl;
    std::cout << "# rank sum = " << check << std::endl;

    /*FILE *fp = fopen("rbbwt.idx", "w") ;
    rbbwt.Save(fp) ;
    fclose(fp) ;*/
  } 
  
  free(abList) ;
  return 0 ;
}
