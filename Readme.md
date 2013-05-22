The H++ Compiler
================

# What is the H++ Language?

Is a language created just for fun, and is pretty similar to C/C++, Java or C# (A mix of them).
This is a native non-optimizing compiler, written in the old-school style, that generates 
assembler x86.

After compiling the H++ compiler, using Visual C++ 2008 or later, you'll need to download the MASM 6.5, 
because H++ relies on ML and LINK32 to compile a full native binary to run on a x86 32 bits computer.

# Source Code Organization

## The root folder 

The root / contains the following H++ compiler's components:

* The main file HCC
* The Lexer
* The Parser
* The Code Generator
* The Source Unit Compactor
* The Assembly Source Unit 

## The HCC Library folder (HCCLib)

This folder contains the following components:

* The Intermediate Code Generator
* The Core Listing Component
* The Error/Warning Manager
* The Custom Iterator component

## The HCC Assembly Library folder (HCCAsmLib) 

This folder contains the standard H++ library written in Assembly x86 32-bits. These library files
are:

* hcclib32.asm
* hcclib32.INC
* SmallWin.inc

## The H++ standard library file stdapi.hcc

This file must be imported in every H++ source code, and this file is located in /TestSource/stdhpp folder.

## The UnitTesting folder called TestSource

This folder contains all the unit testing files created for this project. The main unit testing files are:

* UnitTesting1.hpp
* Algorithms.Sorting.hpp
* ExpMath.hpp
* StanfordAlgs.hpp
* TestMatrices.hpp
* TestSorting.hpp
* NC_IEEE.hpp
* stdhpp/TestFunctionParams.hpp
* stdhpp/TestVirtuals.hpp

The C++ unit testing for the Custom iterator is found in /HCCTools.

# Compiling the H++ compiler 

You'll need a C/C++ compiler like Visual C++ 2008 or later, and before compiling the compiler, install the STL Port 5.2 library. 
After compiling H++, you must run tests around the unit testing files. See the next section.

Remember to install MASM 6.5 before running the HCC exe file to compile the HPP source files.

# Support PDF files

The folder /Intel x86 Manuals, contains the Intel x86 Manuals for supporting the library development.

# Compiling the UnitTesting file

You can compile the unit testing files running the H++ like this:

        hcc unittesting1.hpp /cl /w /GH /GZ /A /Oy

### Example

This is a typical compilation of unittesting1.hpp:

        \HCC\HCC\TestSource>hcc unittesting1.hpp /cl /w /GH /GZ /A /Oy
        H++ Compiler (R) by Harold L. Marzan Version 1.0.0.2074 (Beta)
        Copyright (C) Harold Marzan, 2003-2013. All rights reserved.
        
        Compiling...
        unittesting1.hpp
        importing: stdapi.hcc
                         518 source lines.
                           0 syntax errors.
        >unittesting1.hpp:
        importing: testvirtuals.hpp
                         143 source lines.
                           0 syntax errors.
        >unittesting1.hpp:
        importing: testfunctionparams.hpp
                         106 source lines.
                           0 syntax errors.
        >unittesting1.hpp:
        importing: testswitch.hpp
                         363 source lines.
                           0 syntax errors.
        >unittesting1.hpp:
                        1723 source lines.
                           0 syntax errors.
        Generating Code...
         Assembling: unittesting1.asm
         Assembling: stdhpp\hcclib32.asm
        Linking...
        unittesting1.exe - 0 error(s), 0 warning(s).
        
        Compilation time: 1.6 secs.
        
        \HCC\HCC\TestSource>

### H++ Currently Supported Options

This are the actual supported options in the H++ compiler:

        \HCC\HCC\TestSource>hcc /?
        H++ Compiler (R) by Harold L. Marzan Version 1.0.0.2074 (Beta)
        Copyright (C) Harold Marzan, 2003-2013. All rights reserved.
        
        Usage: hcc <source file> [options]
        
        Compiler Options:
        /x              -display symbolic cross reference
        /c              -compile only, no link (default)
        /cl             -compile and Link
        /nologo         -suppress copyright message
        /w              -disable all warnings
        /I:[search-path] -add import search path for hpp sources
        
        /Fa:[file]      -name assembly listing file
        /Fo:[file]      -name object file
        /Fe:[file]      -name executable file
        /GZ             -use 'CCh' pattern for initialization of stack variables
        /GH             -use 'CDh' pattern for initialization of heap variables
        /FS:[size]      -specifies the total stack allocation in virtual memory for a function (1 MB default)
        /Ox             -specifies a size optimization for function prolog and epilog
        /Oy             -specifies a size optimization for function epilog only
        /A              -add source annotation to the generated assembly code
        /Fvd            -forces converting to virtual destructors when declared non-virtuals
        
        /L[console|windows]     -creates a Console|Windows application
        /Vl             -verbose output while linking. Use with /cl option.
        /Pdb:[file]             -generate a Program Database for debugging symbols
        /S              -show H++ sources for every translation unit while compiling
        /?              -show this usage help
