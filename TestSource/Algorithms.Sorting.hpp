import "stdhpp\stdapi.hcc";




namespace Algorithms
{
	namespace Sorting
	{
		typename unsigned int __uint;
		//base class for all sorting algorithms implementation
		class Sorter
		{
			protected virtual void Swap(double [] array, __uint a, __uint b)
			{
				double tmp 	= array[a];
				array[a] 	= array[b];
				array[b] 	= tmp;
			}
			protected virtual void SwapValues(double ref _A, double ref _B)
			{
				double tmp 	= _A;
				_A = _B;
				_B = tmp;
			}
			
			public virtual abstract void Sort(double [] array, unsigned int size);
		};
		
		
		/* 	Algorithm: S E L E C T I O N   S O R T  - This has a complexity of O(n^2)
			Pros: Simple and easy to implement
			Cons: Inefficient for large lists, so similar to the more efficient Insertion Sort in where the insertion sort should be used in its place.
			Recommendation: use the Insertion Sort instead, when items are n >1000.
		*/
		class SelectionSort : Sorter
		{
			public virtual void Sort(double [] array, unsigned int size)
			{
				for(__uint i=0; i < size; i++)
				{
					int _min = i;
					//find the minimun in the list from i+1...
					for(__uint j = i+1; j< size; j++)
					{
						if(array[j] < array[_min])
							_min = j;
					}
					/*
					double tmp 	= array[i];
					array[i] 	= array[_min];
					array[_min] = tmp;
					*/
					Swap(array, i, _min);
				}
			}
		};
		
		/* 	Algorithm: B U B B L E   S O R T  - This has a complexity of O(n^2)
			Pros: Simple and easy to implement
			Cons: The most inefficient sorting algorithm in use; the worst time overall
			Recommendation: use the Insertion Sort instead for any n > 0
		*/		
		class BubbleSort : Sorter
		{
			public virtual void Sort(double [] array, unsigned int size)
			{
				for(__uint i = (size - 1); i >=0; i--)
				{
					for(__uint j = 1; j <= i; j++)
					{
						if(array[j] < array[j-1])
						{
							/*
							double tmp 	= array[j-1];
							array[j-1] 	= array[j];
							array[j] 	= tmp;
							*/
							Swap(array, j-1, j);
						}
					}
				}
			}
		};
		
		/* 	Algorithm: I N S E R T I O N   S O R T  - This has a complexity of O(n^2)
			Pros: Simple and easy to implement relatively - twice as fast as the Bubble Sort and almost 40% faster than the Selection Sort
			Cons: Inefficient for large lists
			Recommendation: use it when n <= 16
		*/		
		
		class InsertionSort : Sorter
		{
			public virtual void Sort(double [] array, unsigned int size)
			{
				for(__uint i = 1; i < size; i++)
				{
					double dIndex = array[i];
					__uint j = i;
					while(j > 0 && array[j-1] > dIndex)
					{
						array[j] = array[j-1];
						j--;
					}
					array[j] = dIndex;
				}
			}
		};

		/* 	Algorithm: S H E L L   S O R T  - This has a complexity of O(n^2) - Created by Donald Shell in 1959
			Pros: Efficient for medium-size lists. The fastest of all O(n^2) sorting algorithms. Rely on the Insertion Sort to sort a set of sub-lists in the array
			Cons: Complex implementation; not efficient as MergeSort, HeapSort, and QuickSort
			Recommendation: use it when:  (0 < n < 5000)
		*/				
		class ShellSort : Sorter
		{
			public virtual void Sort(double [] array, unsigned int size)
			{
				__uint inc = 3;
				while(inc > 0)
				{
					for(__uint i = 0; i < size; i++)
					{
						__uint j = i;
						double tmp = array[i];
						while((j >= inc) && (array[j - inc] > tmp))
						{
							array[j] = array[j - inc];
							j = j - inc;
						}
						array[j] = tmp;
					}
					if(inc div 2 != 0)
						inc = inc div 2;
					else if(inc == 1)
						inc = 0;
					else
						inc = 1;
				}
			}
		};
		
		/* 	Algorithm: Q U I C K   S O R T  - This has a complexity of O(n^2) for the worst case, and  O(n lg n) for the best case scenario - Created by A.C Hoare. Divide-and-Conquer Algorithm.
			Pros: Extremely fast. This is often used combined with Insertion Sort, to provide an hybrid that can work for smallest and largest lists possible. Best case is O(n lg n)
			Cons: Complex implementation; complex algorithm, massively recursive. Worst case is O(n^2)
			Recommendation: use it when:  (25 < n  < x). Use it if speed is important, and not the # of comparisons and # of data moves.
		*/				
		
		class QuickSort : Sorter
		{
			protected void Partition(double [] array, __uint ref _F, __uint ref _L, double dPivot, bool bPartitionRight)
			{
				if(bPartitionRight)
				{
					while(array[_L] >= dPivot && (_F < _L))
						_L--;
				}else{
					while(array[_F] <= dPivot && _F < _L)
						_F++;				
				}
				
				if(_F!=_L)
				{
					if(bPartitionRight)
					{
						array[_F] = array[_L];
						_F++;
					}else{
						array[_L] = array[_F];
						_L--;					
					}
				}

			}
			protected void quickSort(double [] array, __uint _F, __uint _L)
			{
				__uint F_Hold = _F;
				__uint L_Hold = _L;
				double dPivot = array[_F];
				while(_F < _L)
				{
					Partition(array, _F, _L, dPivot, true);
					Partition(array, _F, _L, dPivot, false);
				}
				array[_F] = dPivot;
				__uint pivot = _F;
				_F = F_Hold;
				_L = L_Hold;
				if(_F < pivot)
					quickSort(array, _F, pivot - 1);
				if(_L > pivot)
					quickSort(array, pivot + 1, _L);
				
			}
			
			public virtual void Sort(double [] array, unsigned int size)
			{
				quickSort(array, 0, size - 1);
				return;
			}
		};
		
		/* 	Algorithm: M E R G E   S O R T  - This has a complexity of O(n lg n) 
			Pros: Marginally faster than the HeapSort for larger sets
			Cons: Complex implementation; complex algorithm, and at least twice the memory requirements of the other sorts; recursive.
			Recommendation: use it when:  (25 < n  < x). Use QuickSort if memory is important, or the HeapSort for larger sets
		*/				
		
		class MergeSort : Sorter
		{
			//_F = First Or Left
			//_L = Last Or Right
			//_M = Middle
			protected void doMerge(double [] array, double [] temp, __uint _F, __uint _M, __uint _L)
			{
				__uint _F_End 		= _M - 1,
				       tmp_pos 		= _F,
					   item_count 	= _L - _F + 1;
				
				while(_F <= _F_End && _M <= _L)
				{
					if(array[_F] <= array[_M]){
						temp[tmp_pos] = array[_F];
						tmp_pos++;
						_F++;
					}else{
						temp[tmp_pos] = array[_M];
						tmp_pos++;
						_M++;
					}
				}
				while(_F <= _F_End){
					temp[tmp_pos] = array[_F];
					tmp_pos++;
					_F++;
				}
				while(_M <= _L){
					temp[tmp_pos] = array[_M];
					tmp_pos++;
					_M++;
				}
				
				for(__uint index=0; index < item_count; index++)
				{
					array[_L] = temp[_L];
					_L--;
				}
			}
		
			protected void mergeSort(double [] array, double [] temp, __uint _F, __uint _L)
			{
				if(_L > _F)
				{
					__uint _M = (_L + _F) div 2;
					mergeSort(array, temp, _F, _M); 	//left half
					mergeSort(array, temp, _M + 1, _L); //right half
					
					doMerge(array, temp, _F, _M + 1, _L); //do the hard work!
				}
				return;
			}
						
			public virtual void Sort(double [] array, unsigned int size)
			{
				double [] temp = new double[size];
				mergeSort(array, temp, 0, size - 1);
				destroy [] temp;
			}
		};
		
		/* 	Algorithm: H E A P   S O R T  - This has a complexity of O(n lg n) . This is the slowest of O(n lg n)
			Pros: In-place and not recursive, making it a good choice for extremely large data sets.
			Cons: Complex implementation; complex algorithm, slower than Merge and Quick sorts.
			Recommendation: use it when:  (25 < n  < x). Use QuickSort if memory is important, and this one for larger data sets.
		*/				
		
		class HeapSort : Sorter
		{
			protected void siftDown(double []array, __uint _Root, __uint _Bottom)
			{
				__uint maxChild = 0;
				
				while(_Root*2 <= _Bottom)
				{
					if(_Root*2 == _Bottom)
						maxChild = _Root * 2;
					else if(array[_Root * 2] > array[_Root * 2 + 1])
						maxChild = _Root * 2;
					else
						maxChild = _Root * 2 + 1;
						
					if(array[_Root] < array[maxChild])
					{
						//swap these values...
						/*
						double tmp 		= array[_Root];
						array[_Root] 	= array[maxChild];
						array[maxChild] = tmp;
						*/
						Swap(array, _Root, maxChild);
						_Root 			= maxChild;
						continue;
					}
					
					break;
				}
			}
			
			public virtual void Sort(double [] array, unsigned int size)
			{
				for(__uint i = (size div 2)- 1; i >= 0; i--)
					siftDown(array, i, size);
					
				for(i = size-1; i >= 1; i--)
				{
					//swap these values
					/*
					double tmp 	= array[0];
					array[0] 	= array[i];
					array[i] 	= tmp;
					*/
					SwapValues(array[0], array[i]);
					//Swap(array, 0, i);
					siftDown(array, 0, i-1);
				}
			}
		};
		
		class BinaryInsertionSort : Sorter
		{
			public virtual void Sort(double [] array, unsigned int size)
			{
				for(__uint i = 1; i < size; ++i)
				{
					double tmp = array[i];
					__uint _F = 0;
					__uint _L = i;
					while(_F < _L)
					{
						__uint _M = (_F + _L) div 2;
						if(tmp >= array[_M])
							_F = _M + 1;
						else
							_L = _M;
					}
					for(__uint j = i; j > _F; --j)
						SwapValues(array[j-1], array[j]);
						//Swap(array, j - 1, j);
				}
			}
		};
		
		namespace Proxy
		{
			class ProxyObject
			{
				public virtual abstract int Compare(ProxyObject ^ obj);
			};
		
			//the base class Array
			class Array
			{
				protected ProxyObject^ array 	= null;
				protected __uint _length 		= 0;
				protected __uint _item_size 	= 0;
				
				public __uint get:length()
					{return _length;}
				public __uint get:item_size()
					{return _item_size;}
					
				public virtual abstract void Swap(__uint left, __uint right);
				public virtual abstract ProxyObject^ getAt(__uint pos);
				
				public virtual abstract void CopyTo(Array ref toArray, __uint to, __uint from);
				public virtual abstract void CopyFrom(Array ref fromArray, __uint from, __uint to);
				
				public virtual void Destructor()
				{
					if(array!=null)
					{	
						destroy [] array;
						array = null;
					}
				}
			};
			
			class Automobile : ProxyObject
			{
				private double _cost 			= 0.0;
				private string _model 			= "";
				private unsigned short _year 	= 0;
				
				public double get:Cost()
					{return _cost;}
				public void put:Cost(double val)
					{_cost = val;}
					
				public string get:Model()
					{return _model;}
				public void put:Model(const string model)
					{_model = model;}
				
				public unsigned short get:Year()
					{return _year;}
				public void put:Year(unsigned short year)
					{_year = year;}
					
				public Automobile(){} //this default constructor is need when using the new operator
				
				public virtual int Compare(ProxyObject ^ obj)
				{
					Automobile^ pobj = dynamic_cast(obj);
					//first, attempt to compare the cars by year, then by cost to determine the lesser
					if(_year < pobj._year && _cost <= pobj._cost)
						return -1;
					else if(_year > pobj._year && _cost >= pobj._cost)
						return 1;
					//finally, determine the car with less price...
					if(_cost < pobj._cost)
						return -1;
					else if(_cost > pobj._cost)
						return 1;
						
					return 0;
				}
				
				public virtual void Destructor()
				{
					Console::WriteLn("Destroying an auto...");
				}
			};
			
			class ArrayOfAutomobile : Array
			{
				public ArrayOfAutomobile(__uint len)
				{
					_item_size = sizeof(Automobile);
					array = new Automobile[_length = len];
				}
				
				public virtual void Swap(__uint left, __uint right)
				{
					/*TODO : allow these type of expressions
					Automobile tmp = array[left];
					array[left] = array[right];
					array[right] = tmp;
					*/
					double cost = array[left].Cost;
					short year = array[left].Year;
					string model = array[left].Model;
					
					array[left].Cost = array[right].Cost;
					array[left].Year = array[right].Year;
					array[left].Model = array[right].Model;
					
					
					array[right].Cost = cost;
					array[right].Year = year;
					array[right].Model = model;
					
				}
				
				public virtual ProxyObject^ getAt(__uint pos)
				{
					if(pos > _length)
						return null;
						
					return &array[pos];
				}
				
				public virtual void CopyTo(Array ref toArray, __uint to, __uint from)
				{
					ProxyObject^ toObject 	= toArray.getAt(to);
					Automobile^ toAuto 		= dynamic_cast(toObject);

					toAuto.Cost 	= array[from].Cost;
					toAuto.Year 	= array[from].Year;
					toAuto.Model 	= array[from].Model;
				}
				public virtual void CopyFrom(Array ref fromArray, __uint from, __uint to)
				{
					ProxyObject^ fromObject = fromArray.getAt(from);
					Automobile^ fromAuto	= dynamic_cast(fromObject);
					
					array[to].Cost 	= fromAuto.Cost;
					array[to].Year 	= fromAuto.Year;
					array[to].Model = fromAuto.Model;
				}
			};
			
			//class Sorter base class for objects | custom types
			class SorterEx
			{
				protected Array^ _array = null;
				public virtual abstract void Sort(Array ref array);
				protected virtual void Swap(__uint left, __uint right)
				{
					if(_array!=null)
						_array.Swap(left, right);
				}
			};
			
			//Insertion Sort: O(n^2)
			class InsertionSortEx : SorterEx
			{
				public virtual void Sort(Array ref array)
				{
					//for use in the Swap member...
					_array = &array; //the Swap member requires a pointer to the array
					__uint n = array.length;
				
					for(__uint i = 1; i < n; ++i)
						for(__uint j = i; j > 0; --j)
						{
							if(array.getAt(j - 1).Compare(array.getAt(j)) > 0)
								Swap(j, j - 1);
						}
				}	
			};
			
			//Binary Insertion Sort: O(n log n)
			//_F = first | left
			//_L = last | right
			//_M = middle
			class BinaryInsertionSortEx : SorterEx
			{
				public virtual void Sort(Array ref array)
				{
					_array = &array; //the Swap member requires a pointer to the array
					__uint n = array.length;
					for(__uint i = 1; i < n; ++i)
					{
						ProxyObject^ tmp = array.getAt(i);
						__uint _F = 0;
						__uint _L = i;
						while(_F < _L)
						{
							__uint _M = (_F + _L) div 2;
							if(tmp.Compare(array.getAt(_M)) >= 0)
								_F = _M + 1;
							else
								_L = _M;
						}
						for(__uint j = i; j > _F; --j)
							Swap(j - 1, j);
					}
				}
			};
			
			//Bubble Sort : O(n^2)
			class BubbleSortEx : SorterEx
			{
				public virtual void Sort(Array ref array)
				{
					_array = &array; //the Swap member requires a pointer to the array
					__uint n = array.length;
					for(__uint i = n; i > 1; --i)
						for(__uint j = 0; j < i - 1; ++j)
							if(array.getAt(j).Compare(array.getAt(j + 1)) > 0)
								Swap(j, j + 1);
				}
			};
			
			//Quick Sort: O(n log n)
			class QuickSortEx : SorterEx
			{
				protected static __uint cutOff = 1;
				protected virtual abstract __uint selectPivot(Array ref array, __uint _F, __uint _L);
				protected virtual void doSort(Array ref array, __uint _F, __uint _L)
				{
					if(_L - _F + 1  > cutOff)
					{
						__uint _Piv = selectPivot(array, _F, _L);
						Swap(_Piv, _L);
						_Piv = _L;
						ProxyObject^ pivot = array.getAt(_Piv);
						__uint i = _F;
						__uint j = _L - 1;
						for(;;)
						{
							while(i < j && array.getAt(i).Compare(pivot) < 0) ++i;
							while(i < j && array.getAt(i).Compare(pivot) > 0) --j;
							if(i >= j) break;
							Swap(i++, j--);
						}
						if(array.getAt(i).Compare(pivot) > 0)
							Swap(i, _Piv);
						if(_F < i)
							doSort(array, _F, i - 1);
						if(_L > i)
							doSort(array, i + 1, _L);
					}
				}				
				public virtual void Sort(Array ref array)
				{
					_array = &array; //the Swap member requires a pointer to the array
					doSort(array, 0, array.length - 1);
					InsertionSortEx sorter;
					sorter.Sort(array);
				}
			};
			
			class MedianOfThreeQuickSort : QuickSortEx
			{
				protected virtual __uint selectPivot(Array ref array, __uint _F, __uint _L)
				{
					__uint _M = (_F + _L) div 2;
					if(array.getAt(_F).Compare(array.getAt(_M)) > 0)
						Swap(_F, _M);
					if(array.getAt(_M).Compare(array.getAt(_L)) > 0)
						Swap(_M, _L);						
					if(array.getAt(_F).Compare(array.getAt(_L)) > 0)
						Swap(_F, _L);
					return _M;
				}
			};
			
			//Selection Sort: O(n^2)
			class SelectionSortEx : SorterEx
			{
				public virtual void Sort(Array ref array)
				{
					_array = &array; //the Swap member requires a pointer to the array
					for(__uint i = array.length; i > 1; --i)
					{
						__uint max = 0;
						for(__uint j = 1; j < i; ++j)
							if(array.getAt(j).Compare(array.getAt(max)) > 0)
								max = j;
						Swap(i - 1, max);
					}
				}
			};
			
			//Heap Sort: O(n log n)			
			class HeapSortEx : SorterEx
			{
				protected void siftDown(Array ref array, __uint _Root, __uint _Bottom)
				{
					__uint maxChild = 0;
					
					while(_Root*2 <= _Bottom)
					{
						if(_Root*2 == _Bottom)
							maxChild = _Root * 2;
						else if(array.getAt(_Root * 2).Compare(array.getAt(_Root * 2 + 1)) > 0)
							maxChild = _Root * 2;
						else
							maxChild = _Root * 2 + 1;
							
						if(array.getAt(_Root).Compare(array.getAt(maxChild)) < 0)
						{
							//swap these values...
							Swap(_Root, maxChild);
							_Root = maxChild;
							continue;
						}
						
						break;
					}
				}
				
				public virtual void Sort(Array ref array)
				{
					_array = &array; //the Swap member requires a pointer to the array
					
					for(__uint i = (array.length div 2) - 1; i >= 0; i--)
						siftDown(array, i, array.length);
						
					for(i = array.length - 1; i >= 1; i--)
					{
						//swap these values
						Swap(0, i);
						siftDown(array, 0, i - 1);
					}
				}
			};
			
			//Two-way Merge Sort : O(n log n)
			class TwoWayMergeSort : SorterEx
			{
				protected Array^ tmpArray = null;
				protected void Merge(Array ref array, __uint _F, __uint _M, __uint _L)
				{
					__uint i = _F;
					__uint j = _F;
					__uint k = _M + 1;
					while(j <= _M && k <= _L)
					{
						if(array.getAt(j).Compare(array.getAt(k)) <= 0)
							array.CopyTo(tmpArray, i++, j++);
						else
							array.CopyTo(tmpArray, i++, k++);						
					}	
					while(j <= _M)
						array.CopyTo(tmpArray, i++, j++);
					while(k <= _L)
						array.CopyTo(tmpArray, i++, k++);
					for(i = _F; i <= _L; ++i)
						array.CopyFrom(tmpArray, i, i);	
				}
				
				protected void doSort(Array ref array, __uint _F, __uint _L)
				{
					if(_F < _L)
					{
						__uint _M = (_F + _L) div 2;
						doSort(array, _F, _M);
						doSort(array, _M + 1, _L);
						Merge(array, _F, _M, _L);
					}
				}
				
				public virtual void Sort(Array ref array)
				{
					tmpArray = new ArrayOfAutomobile(array.length); //ADHOC : might be in t he future:  
					//tmpArray = pointer_cast(malloc(array.length * array.item_size));
					doSort(array, 0, array.length - 1);
					destroy tmpArray;
					tmpArray = null;
				}
			};
		};
	};
};