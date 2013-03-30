import "stdhpp\stdapi.hcc";

namespace UnitTesting //TestSwitch.hpp
{
	class TestSwitch
	{
		public const int open  = 1;
		public const int close = 2;
		public const int write = 3, 
						 read = 4;

		public static void RunTest(int option)
		{	
			switch(option)
			{
			case open:
				Console::WriteLn("Opening file...");
				break;
			case close:
				Console::WriteLn("Closing file...");
				break;
			case write:
				Console::WriteLn("Do Write to file...");
				break;
			case read:
				Console::WriteLn("Do Read from file...");
				break;
			case 5:
				Console::WriteLn("Do for 5 and follow to 6");
			case 6:
				{
				Console::WriteLn("Do 6, then finish");
				}
				break;
			default:
				Console::WriteLn("Invalid Option : ", option);
			};
		}
	};

	typename enum ShapeType
	{
		stCircle = 1,
		stSquare,
		stTriangle,
	}ShapeType;

	class Shape
	{		
		public virtual abstract ShapeType Type();
		public virtual abstract double ComputeArea();		
		public virtual abstract string Name();
		public virtual abstract void Draw();
		//virtual destructor
		public virtual abstract void Destructor();
	};

	class Circle: Shape
	{
		private double radius = 0.0;
		
		public Circle()
		{
		}

	public double get:Radius()
	{
		return radius;
	}

	public void put:Radius(double value)
	{
		radius = value;
	}

	protected double Circumference()
	{
		return Math::pi() * Radius;
	}

		public virtual ShapeType Type()
		{
			return stCircle;
		}		
		public virtual string Name()
		{
			return "Circle";
		}

		public virtual void Draw()
		{
			Console::WriteLn("Drawing a Circle!");
		}

		public virtual double ComputeArea()
		{
			return 2 * Circumference();
		}
		
		public void Destructor()
		{
			System::Debug::OutputString("destroying a Circle object...");
			Console::WriteLn("destroying a Circle object...");
		}
		
		public virtual void DrawSphere()
		{
			Console::WriteLn("Drawing a Sphere!");
		}		
	};

	class Square: Shape
	{
		private double dWidth = 0.0;
		private double dHeight = 0.0;
		
		public Square()
		{}
		
		public virtual void Destructor()
		{
			System::Debug::OutputString("destroying a Square object...");
			Console::WriteLn("destroying a Square object...");
		}
		

	public double get:Width()
	{
		return dWidth;
	}

	public void put:Width(double value)
	{
		dWidth = value;
	}

	public double get:Height()
	{
		return dHeight;
	}

	public void put:Height(double value)
	{
		dHeight = value;
	}

	public virtual double ComputeArea()
	{
		return Width * Height;
	}
		public virtual ShapeType Type()
		{
			return stSquare;
		}		
		public virtual string Name()
		{
			return "Square";
		}

		public virtual void Draw()
		{
			Console::WriteLn("Drawing a Square!");
		}

		public void DrawCube()
		{
			Console::WriteLn("Drawing a Cube!");
		}
	};

	class Triangle: Shape
	{
		private double dBase = 0.0;
		private double dHigh = 0.0;
		
		public Triangle()
		{}

	public double get:Base()
	{
		return dBase;
	}

	public void put:Base(double value)
	{
		dBase = value;
	}

	public double get:High()
	{
		return dHigh;
	}

	public void put:High(double value)
	{
		dHigh = value;
	}
		public virtual ShapeType Type()
		{
			return stTriangle;
		}		
		public virtual string Name()
		{
			return "Triangle";
		}

		public virtual void Draw()
		{
			Console::WriteLn("Drawing a Triangle!");
		}


		public virtual double ComputeArea()
		{
			return Base * High / 2;
		}
	
		public void Destructor()
		{
			System::Debug::OutputString("destroying a Triangle object...");
			Console::WriteLn("destroying a Triangle object...");
			return;
		}
		
		public virtual void DrawPiramid()
		{
			Console::WriteLn("Drawing a Piramid!");
		}
	};
	
	class ShapeFactoryCreator
	{
	/*
		public static Circle^ test1 = null;
		public Circle^ test2 = null;
		
		private Shape^ ptr1 = null;
		*/
		//private Shape^ ptr2 = new Shape();  //must fail (abstract)
		
		private Shape^ ptr3 = new Circle();
		
		public static Shape^ Create(const string name)
		{
			Shape^ obj_ptr = new Circle();
			
			//Shape^ obj2 = new Shape();  //must fail (abstract)
			//Shape obj3;  //must fail (abstract)
			//Shape obj4();  //must fail (abstract)
			
			destroy dynamic_cast(Circle, obj_ptr);
	
/*
			Circle^ cl_ptr = dynamic_cast(obj_ptr);
			
			cl_ptr = dynamic_cast(obj_ptr);
			
			destroy cl_ptr; //destroys a Circle instance
	*/		
/*
			obj_ptr = new Triangle();
			
			Triangle^ tr_ptr = dynamic_cast(obj_ptr);
			
			tr_ptr = dynamic_cast(obj_ptr);
			
			destroy tr_ptr; //destroys a Triangle instance
			
			obj_ptr = new Square();
			
			Square^ sq_ptr = dynamic_cast(obj_ptr);
			
			sq_ptr = dynamic_cast(obj_ptr);
			
			destroy sq_ptr; //destroys a Square instance
*/			
			if(name=="Circle")
			{
				return new Circle();
			}else if(name=="Square")
			{
				return new Square();
			}else if(name=="Triangle")
			{
				return new Triangle();
			}
		
			return null;
		}
		
		public void Destructor()
		{
			/*
			Circle^ cl_ptr = dynamic_cast(ptr3);
			
			destroy cl_ptr; //destroys a Circle instance
			*/
			 //TODO: we must clone the type-spec to flag a pointer is being compared
			if(ptr3!=null)
				destroy ptr3;
				
			if(null!=ptr3)
				destroy ptr3;
				
		}
	};
	
	class Visitor
	{
		public void Visit(Shape^ shape)
		{
			shape.Draw();
			
			Shape^ shape_ptr = null;			
			
			switch(shape.Type())
			{
			case stCircle:
			{
				Console::WriteLn("type == stCircle");
				
				Circle^ circle = dynamic_cast(shape);
				circle.DrawSphere(); //virtual
				
				shape_ptr = circle; //upcast
			}
			break;
			case stSquare:
			{
				Console::WriteLn("type == stSquare");
				
				Square^ square = dynamic_cast(shape);
				square.DrawCube();
				
				shape_ptr = square; //upcast
			}
			break;
			case stTriangle:
				{
					Console::WriteLn("type == stTriangle");
					
					Triangle^ triangle = dynamic_cast(shape);
					triangle.DrawPiramid();
					
					shape_ptr = triangle; //upcast
				}
			break;
			default:
				debugger; //should not get here!
			break;
			};
						
			if(shape_ptr!=null)
				shape_ptr.Draw();
				
			if(null!=shape_ptr)
				shape_ptr.Draw();	
				
				
			Console::WriteLn("**Visited object name = ", shape_ptr.Name());
		}
	};
}