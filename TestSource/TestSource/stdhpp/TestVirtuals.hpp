import "TestSource\stdhpp\stdapi.hcc";

namespace UnitTesting //TestVirtuals.hpp
{
	class Rnd
	{
		private static long seed = 1;
		public static long a = 16807;
		public static long m = 2147483647;
		public static long q = 127773;
		public static long r = 2836;

		public Rnd(int seedx)
		{
			UnitTesting::Rnd::seed = seedx;
		}

		public void setSeed(long s)
		{
			//using the fully qualified name...
			UnitTesting::Rnd::seed = s;
		}

		public static double NextDblEx()
		{			
			Rnd::seed = Math::Round(Rnd::a * (Rnd::seed % Rnd::q) - Rnd::r * (Rnd::seed * Rnd::q));

			Rnd::seed += (seed < 0 ? Rnd::m : 0);

			return Rnd::seed / Rnd::m;
		}

		public static double NextIntEx()
		{			
			//Rnd::seed = Math::Round(a * (seed % q) - r * (seed * q));
			Rnd::seed = a * (seed % q) - r * (seed * q);

			Rnd::seed += (seed < 0 ? Rnd::m : 0);

			return UnitTesting::Rnd::seed % Rnd::m;
		}

		public double NextDbl()
		{
			return Rnd::NextDblEx();
		}

		public double NextInt()
		{
			return Rnd::NextIntEx();
		}
	};

	namespace TestVirtuals
	{
		//the base class
		class RandomVariable
		{
			public virtual abstract double Sample();
			public virtual string Name()
			{
				return "(none)";
			}
		};

		class SimpleRV : RandomVariable
		{
			public virtual double Sample()
			{
				return Rnd::NextDblEx();
			}

			public virtual string Name()
			{
				return "A simple variable";
			}
		};

		class UniformRV: RandomVariable
		{
			private double u;
			private double v;

		public double get:U()
		{
			return u;
		}

		private void put:U(double value)
		{
			u = value;
		}

		public double get:V()
		{
			return v;
		}

		private void put:V(double value)
		{
			v = value;
		}

			public UniformRV(double _u, double _v)
			{
				U = _u; V = _v;
			}

			public double Sample()
			{
				return u + (v - u) * Rnd::NextDblEx();
			}
			public virtual string Name()
			{
				return "An uniform variable";
			}
		};

		class ExponentialRV : RandomVariable
		{
			private double mu;

		public double get:MU()
		{
			return mu;
		}

		public ExponentialRV(double _mu)
		{
			mu = _mu;
		}
			public virtual double Sample()
			{
				return -MU * Math::log10(Rnd::NextDblEx());
			}
			public string Name()
			{
				return "An exponential variable";
			}

		};
	}
}