import "stdhpp\stdapi.hcc";

namespace Test
{
	int nVal;
	static double dYVal = 0.0;

	class HelloWorld	//the only class we have for this test
	{
		public static double dXVal = 0.0;
		public double Hello2(int i, int ref j, double dYVal)	//
		{
			j = 5;
			return (i + dYVal + 0.5);
		}

		public void Hello1(int i, int ref j) //this is a great day me!
		{
			//we have 56 bytes in local variables
			//and we have 4 more for the this pointer...
			double result_time = 0;

			int a = 1;
			int b = 4;
			int c = 4;

			double disc = 0.0;

			double uno = 1.0;

			uno = 7.14 * 1.0;

			uno = 7.14 * (-1.0);

			int speed		= 120; //miles per hour
			double distance = 144; //miles

			//v = d/t
			result_time = distance / speed;

			double dSpeed = distance / result_time;

//			System::Debug::BreakPoint();
			//we must have as the stack result, the last expression
			(result_time = distance / dSpeed, dSpeed = distance / result_time, disc = Math::Sqr(b) - 4 * a * c);

			Console::WriteLn("The resultant time is t = ", result_time);
			Console::WriteLn("The resultant speed is s = ", dSpeed);
			Console::WriteLn("The resultant discriminant is s = ", disc);

			
			j = i div 2;
			Console::WriteLn("Here, we have j = ", j);

			/*
				The Visual Studio IDE, is the best in the world
			*/

			dXVal = Hello2(i, j, 3.14);
			dYVal = Hello2(i, j, 3.14);
			Console::WriteLn("Here, now we have j = ", j);
		}


		
		//the entry point function for this little test program...
		public static void main(int argc, string[] argv)
		{
		
			//System::Debug::BreakPoint();
			Console::WriteLn("Number of Parameters = ", argc);
			for(int index=0; index< argc; index++)
			{
				Console::WriteLn("Parameter argv[", index, "]= ", argv[index]);
			}
			
			System::ShowMessage("This is my first H++ program; so I must say 'Hello World'", 
								"Hello World", 
								System::IconInformation);
			nVal = 1;
			Console::WriteLn("Initially, the value of nVal is = ", nVal);
			//we pass nVal address via a reference parameter...

			HelloWorld hello;
			hello.Hello1(7, nVal);
			//now, if we check the value of nVal, must be different, if modified in operation0() function...
			Console::WriteLn("Now, the value of nVal is = ", 
							nVal,
							 " and the value of dXVal is = ", 
							 dXVal);
		}
		
	};
}