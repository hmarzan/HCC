import "stdhpp\stdapi.hcc";
import "stdhpp\TestVirtuals.hpp";
import "TestSwitch.hpp";


using namespace UnitTesting;

namespace LastFeatures
{

using printf = Console::WriteLn;
	
	
	class SimpleObject
	{
		public virtual abstract bool lessThan(SimpleObject ref obj);
		public virtual abstract bool isEqual(SimpleObject ref obj);
		
		public virtual abstract void Swap(SimpleObject ref obj);
		
		public virtual void Destructor()
		{
			printf("Destroying a Simple Object...");
		}
	};
	
	//1
	class DoubleProxy: SimpleObject
	{
	public double val = 0.0;
	
	public DoubleProxy()
	{
		printf("Constructing a Double Proxy...");
	}
		public virtual bool lessThan(SimpleObject ref obj)
		{
			DoubleProxy^ ptr = dynamic_cast(&obj);
			
			return val < ptr.val;
		}
		
		public virtual bool isEqual(SimpleObject ref obj)
		{
			DoubleProxy^ ptr = dynamic_cast(DoubleProxy, &obj);
			
			return val < ptr.val;
		}
		
		public virtual void Swap(SimpleObject ref obj)
		{
			double tmp = val;
			
			DoubleProxy^ ptr = dynamic_cast(&obj);
			val = ptr.val;
			ptr.val = tmp;
		}
	
	};
	
	//3
	class SwapperObject
	{
		public virtual void Swap(DoubleProxy ref obj1, DoubleProxy ref obj2)
		{
	 		//DoubleProxy tmp = obj1; //TODO: implement the Copy Constructor
			DoubleProxy tmp;
			
			tmp = obj1;
			obj1 = obj2;
			obj2 = tmp;
		}
	};
	
	//4
	class SortDoubleProxy
	{
	public void Sort(SimpleObject[] array, int n)
	{
		for(int index=1;index < n; index++)
		{
			for(int left=0; left < index; left++)
			{
				if(array[left + 1].lessThan(array[left]))
				{
						array[left + 1].Swap(array[left]);
				}
			}
		}
	}
	};
	//
	class HObject
	{
		public HObject()
		{
		}
		public virtual void Destructor()
		{
		
		}
	};
	
	class TestProperties
	{
		struct node
		{
			public node^ left = null;
			public node^ right = null;
			public HObject^ value = null;
			
			public node()
			{
			}
			
			public void Destructor()
			{		
				if(left!=null)
					destroy left;
				if(right!=null)
					destroy right;
				if(value!=null)
					destroy value;			
					
			}
		};

		private Shape[] shapes1 = new Circle[3*5];
		
		private Shape[] shapes2 = new Circle[4];
		
		private Shape^ shapes_ptr = new Circle[4*5]; //should this expression change the the decl from pointer to array inmediatelly?
		
		private Circle oshapes[5];
		
		public Shape^ get:Shapes()
		{
			return shapes1;
		}
		
		public void put:Shapes(Shape^ _shape)
		{
		
			node link;
			
			link.left 	= new node();
			link.right 	= new node();
			link.value  = new HObject();
			
			/*TODO : destroy object.member
			destroy link.left;
			destroy link.right;
			destroy link.value;
			*/
			
			node^ left = link.left;
			destroy left;
			link.left = null;
			
			link.left = link.right;
			link.right = new node();
			
			node^ right = link.right;
			destroy right;
			
			link.right = link.left;
			link.left = new node();
			
			left = link.left;
			
			link.left = left;
			
		
			shapes1[0].Draw();
			shapes2[0].Draw();

			shapes_ptr[0].Draw();
			
			shapes1[1].Draw();
			shapes2[2].Draw();
			
			shapes_ptr[3].Draw();
			
			
			
			if(shapes1!=null)
				destroy shapes1; //must use destroy [] shapes for dynamic arrays;
			
			destroy [] shapes1;
			//
			shapes1 = _shape;
			
			destroy [] shapes2;
			
			const int n = 10;
			Circle local_circles[n];
			
			local_circles[0].Draw();
			
			local_circles[2].Draw();
			
			for(int i=0; i < n; i++)
				local_circles[i].Draw();
			
			Circle^ circlxx = new Circle[5];
			
			circlxx[0].Draw();
			
			destroy [] circlxx;
			
			//oshapes = circlxx;
		}		
	};
	
	class TestFeatures
	{
	public static void main(int argc, string[] argv)
	{
	//		debugger;
				for(int ac=0; ac < argc; ac++)
				{
					Console::WriteLn("Argument: [",ac,"]= ", argv[ac]);
				}

debugger;	
				Shape^ obj_ptr = ShapeFactoryCreator::Create("Circle");				
				
				obj_ptr.Draw();
				
				Circle^ circl = dynamic_cast(obj_ptr); //valid
				
				circl = dynamic_cast(Circle, obj_ptr); //valid
				
				//Square^ squar = dynamic_cast(Circle, obj_ptr); //must fail
				//squar = dynamic_cast(Circle, obj_ptr); //must fail
				
				circl.DrawSphere();
				
				Visitor visitor;				
				visitor.Visit(obj_ptr);
				
				destroy dynamic_cast(Circle, obj_ptr); //valid
				
				
				obj_ptr = ShapeFactoryCreator::Create("Circle");				
				
				obj_ptr.Draw();
								
				destroy obj_ptr; //will call the destructor as well

				obj_ptr = ShapeFactoryCreator::Create("Triangle");				
				
				obj_ptr.Draw();
								
				destroy obj_ptr; //will call the destructor as well
				
				obj_ptr = ShapeFactoryCreator::Create("Square");				
				
				obj_ptr.Draw();
								
				destroy obj_ptr; //will call the destructor as well
				

				TestProperties testProp;
				
				Circle^ shapes = new Circle[20];
				testProp.Shapes = shapes;

				/*
				TestProperties test;
				
				test.Shape = new Circle();
				
				destroy test.Shape;
				*/
				
		int hex_value = 0xDEADC0DEh;

		hex_value = hex_value << 0x10; //0xC0DE0000h

		hex_value = hex_value >> 8; //0x00C0DE00h

		hex_value = hex_value >> 8; //0x0000C0DEh
		
		short shift = 0x10;
		
		hex_value << shift; ////0xC0DE0000h

		double^ p = new double;
		
		*p = 3.141516927;
		
		printf("*p (3.1415...)= ", *p);

		double res = *p;
		
		printf("res (3.1415...)= ", res);

		destroy p;
		/*
		p = &res;

		*p = 9.99;
		
		printf("res = (9.99)", res);
		*/
		
		SortDoubleProxy sorter;
		
		int n = 10;
		DoubleProxy [] array = new DoubleProxy[n];
		//TODO: initialization
		array[0].val = 19.2;
		array[1].val = 1.2;
		array[2].val = 6.3456;
		array[4].val = 7.2345;
		array[5].val = 12.4567;
		array[6].val = 9.2345;
		array[7].val = 15.123;
		array[8].val = 3.2345;
		array[9].val = 18.1234;
		sorter.Sort(array, n);
		
		destroy [] array;
		
		return;
	}
	
	};
}