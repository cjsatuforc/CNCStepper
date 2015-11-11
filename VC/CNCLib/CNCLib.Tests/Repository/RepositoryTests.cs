﻿////////////////////////////////////////////////////////
/*
  This file is part of CNCLib - A library for stepper motors.

  Copyright (c) 2013-2015 Herbert Aitenbichler

  CNCLib is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  CNCLib is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  http://www.gnu.org/licenses/
*/

using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using CNCLib.Repository.Context;
using CNCLib.Repository;
using CNCLib.Repository.Entities;
using System.Threading.Tasks;
using Framework.EF;
using System.Collections.Generic;
using System.Linq;
using Framework.Tools;
using CNCLib.Repository.Interfaces;

namespace CNCLib.Tests.Repository
{
	[TestClass]
	public class RepositoryTests
	{
		public TestContext TestContext { get; set; }
		static bool _init = false;
		static protected Framework.Tools.Pattern.IFactory RepositoryFactory { get; set; }

		[ClassInitialize]
		public static void ClassInit(TestContext testContext)
		{
			var factory = new Framework.Tools.Pattern.FactoryType2Type();

			factory.Register(typeof(IConfigurationRepository), typeof(ConfigurationRepository));
			factory.Register(typeof(IMachineRepository), typeof(MachineRepository));

			RepositoryFactory = factory;

			if (_init == false)
			{
				//drop and recreate the test Db everytime the tests are run. 
				AppDomain.CurrentDomain.SetData("DataDirectory", testContext.TestDeploymentDir);

				using (var uow = UnitOfWorkFactory.Create())
				{
					System.Data.Entity.Database.SetInitializer<CNCLibContext>(new CNCLibInitializer());
					uow.InitializeDatabase();
				}
				_init = true;
			}
		}

		[TestInitialize]
		public void Init()
		{
		}
	}
}
