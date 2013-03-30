import "stdhpp\stdapi.hcc";
import "Algorithms.Sorting";

using namespace Algorithms::Sorting;
using namespace Algorithms::Sorting::Proxy;


class Program
{
	public static void TestSortingEx(SorterEx ref sorter, const string title)
	{
		printf(title);
		ArrayOfAutomobile array(9);
		
		ProxyObject^ tmp = array.getAt(0);
		Automobile^ autom = dynamic_cast(tmp);
		
		autom.Year = 2009;
		autom.Cost = 260798.0;
		autom.Model = "Bentley";
		
		tmp = array.getAt(1);
		autom = dynamic_cast(tmp);
		
		autom.Year = 2006;
		autom.Cost = 105000.0;
		autom.Model = "Range Rover";
		
		tmp = array.getAt(2);
		autom = dynamic_cast(tmp);
		
		autom.Year = 2005;
		autom.Cost = 21400.0;
		autom.Model = "Passat";

		tmp = array.getAt(3);
		autom = dynamic_cast(tmp);
		
		autom.Year = 2001;
		autom.Cost = 13900.0;
		autom.Model = "Toyota Camry";
		
		tmp = array.getAt(4);
		autom = dynamic_cast(tmp);
		
		autom.Year = 2008;
		autom.Cost = 150000.0;
		autom.Model = "Hummer";
		
		tmp = array.getAt(5);
		autom = dynamic_cast(tmp);
		
		autom.Year = 2009;
		autom.Cost = 190000.0;
		autom.Model = "Cadillac";
		
		tmp = array.getAt(6);
		autom = dynamic_cast(tmp);
		
		autom.Year = 2007;
		autom.Cost = 1500000.0;
		autom.Model = "Maseratti";
				
		tmp = array.getAt(7);
		autom = dynamic_cast(tmp);
		
		autom.Year = 2004;
		autom.Cost = 17500.0;
		autom.Model = "Honda Civic";		

		tmp = array.getAt(8);
		autom = dynamic_cast(tmp);
		
		autom.Year = 2006;
		autom.Cost = 15500.0;
		autom.Model = "Jetta";		
		
		__uint n = array.length;
		
		printf("\nBefore Sorting...\n");
		for(__uint i=0; i < n; i++)
		{
			tmp = array.getAt(i);
			autom = dynamic_cast(tmp);			
			printf("Auto :\t", autom.Model, "  ,\tYear :\t\t", autom.Year, ", Cost:\t", autom.Cost);
		}
		
		sorter.Sort(array);
		printf("\nAfter Sorting...\n");
		for(i=0; i < n; i++)
		{
			tmp = array.getAt(i);
			autom = dynamic_cast(tmp);			
			printf("Auto :\t", autom.Model, "  ,\tYear :\t\t", autom.Year, ", Cost:\t", autom.Cost);
		}
	}

	public static void TestSortingAlgorithm(Sorter ref sorter, const string title)
	{
		printf(title);
		double array[16];
		
		array[0] = 203.78;
		array[1] = 12.345;
		array[2] = 10.4567;
		array[3] = 687.35;
		array[4] = 5.7556789;
		array[5] = 9.985;
		array[6] = 3.389;
		array[7] = 478.2345;
		array[8] = 0.98754;
		array[9] = 356.2345;
		
		array[0xA] = -1.75;
		array[0xB] = 47.977;
		array[0xC] = -0.67;
		array[0xD] = 4.758;
		array[0xE] = 1.2345;
		array[0xF] = 2.8789;
			
		__uint n = sizeof(array)/sizeof(double);
		
		printf("\nBefore sorting...");
		for(__uint x=0; x < n; x++)
			printf("array[", x, "] = ", array[x]);
		
		sorter.Sort(array, n);
		
		printf("\nAfter sorting...");
		for(x=0; x < n; x++)
			printf("array[", x, "] = ", array[x]);
	}
	
	public static void TestSelectionSort(void)
	{
		SelectionSort sorter;
		TestSortingAlgorithm(sorter, "\nTesting SELECTION SORT Algorithm: O(n^2)");	
	}
	
	public static void TestBubbleSort(void)
	{
		BubbleSort sorter;
		TestSortingAlgorithm(sorter, "\nTesting BUBBLE SORT Algorithm: O(n^2)");	
	}

	public static void TestInsertionSort(void)
	{
		InsertionSort sorter;
		TestSortingAlgorithm(sorter, "\nTesting INSERTION SORT Algorithm: O(n^2) - faster");	
	}
	
	public static void TestShellSort(void)
	{
		ShellSort sorter;
		TestSortingAlgorithm(sorter, "\nTesting SHELL SORT Algorithm: O(n^2) - fastest");	
	}
	
	public static void TestQuickSort(void)
	{
		QuickSort sorter;
		TestSortingAlgorithm(sorter, "\nTesting QUICK SORT Algorithm: worst: O(n^2), best: O(n lg n)");	
	}
	
	public static void TestMergeSort(void)
	{
		MergeSort sorter;
		TestSortingAlgorithm(sorter, "\nTesting MERGE SORT Algorithm: O(n lg n)");	
	}

	public static void TestHeapSort(void)
	{
		HeapSort sorter;
		TestSortingAlgorithm(sorter, "\nTesting HEAP SORT Algorithm: O(n lg n)");	
	}	

	public static void TestBinaryInsertionSort(void)
	{
		BinaryInsertionSort sorter;
		TestSortingAlgorithm(sorter, "\nTesting BINARY INSERTION SORT Algorithm: worst: O(n^2), best: O(n lg n)");
	}

	//O B J E C T   O R I E N T E D   V E R S I O N S 
	
	public static void TestOOInsertionSort(void)
	{
		InsertionSortEx sorter;
		TestSortingEx(sorter, "\nTesting INSERTION SORT Algorithm: O(n^2) - faster");
	}

	public static void TestOOSelectionSort(void)
	{
		SelectionSortEx sorter;
		TestSortingEx(sorter, "\nTesting SELECTION SORT Algorithm: O(n^2)");	
	}
	
	public static void TestOOBubbleSort(void)
	{
		BubbleSortEx sorter;
		TestSortingEx(sorter, "\nTesting BUBBLE SORT Algorithm: O(n^2)");	
	}	

	public static void TestOOBinaryInsertionSort(void)
	{
		BinaryInsertionSortEx sorter;
		TestSortingEx(sorter, "\nTesting BINARY INSERTION SORT Algorithm: worst: O(n^2), best: O(n lg n)");
	}
		
	public static void TestOOQuickSort(void)
	{
		MedianOfThreeQuickSort sorter;
		TestSortingEx(sorter, "\nTesting QUICK SORT Algorithm: worst: O(n^2), best: O(n lg n)");	
	}
	
	public static void TestOOHeapSort(void)
	{
		HeapSortEx sorter;
		TestSortingEx(sorter, "\nTesting HEAP SORT Algorithm: O(n lg n)");	
	}	

	public static void TestOOMergeSort(void)
	{
		TwoWayMergeSort sorter;
		TestSortingEx(sorter, "\nTesting MERGE SORT Algorithm: O(n lg n)");	
	}
	
	public static void main(void)
	{
		printf("\nTest 1:\n");		
		TestSelectionSort();
		printf("\nTest 2:\n");
		TestBubbleSort();
		printf("\nTest 3:\n");
		TestInsertionSort();
		printf("\nTest 4:\n");
		TestShellSort();
		printf("\nTest 5:\n");
		TestQuickSort();
		printf("\nTest 6:\n");
		TestMergeSort();		
		printf("\nTest 7:\n");
		TestHeapSort();		
		printf("\nTest 8:\n");
		TestBinaryInsertionSort();
		
		System::ShowMessage("Now, we are going to test the Objects Sorters", "Sample Sorting Library", System::IconInformation);
		
		TestOOInsertionSort();
		TestOOBubbleSort();
		TestOOSelectionSort();
		TestOOQuickSort();
		TestOOHeapSort();
		TestOOBinaryInsertionSort();
		TestOOMergeSort();
	}
};