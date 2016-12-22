﻿////////////////////////////////////////////////////////
/*
  This file is part of CNCLib - A library for stepper motors.

  Copyright (c) 2013-2016 Herbert Aitenbichler

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

using System.Collections.Generic;
using System.Threading.Tasks;
using CNCLib.Logic.Contracts.DTO;
using CNCLib.ServiceProxy;
using Framework.Tools.Dependency;
using Framework.Web;

namespace CNCLib.WebAPI.Controllers
{
    public class LoadOptionsController : RestController<LoadOptions>
	{
	}

	public class LoadInfoRest : IRest<LoadOptions>
	{
		private ILoadOptionsService _service = Dependency.Resolve<ILoadOptionsService>();

		public async Task<IEnumerable<LoadOptions>> Get()
		{
			return await _service.GetAll();
		}

		public async Task<LoadOptions> Get(int id)
		{
			return await _service.Get(id);
		}

		public async Task<int> Add(LoadOptions value)
		{
			return await _service.Add(value);
		}

		public async Task Update(int id, LoadOptions value)
		{
			await _service.Update(value);
		}

		public async Task Delete(int id, LoadOptions value)
		{
			await _service.Delete(value);
		}

		public bool CompareId(int id, LoadOptions value)
		{
			return true;
		}

		#region IDisposable Support
		private bool _disposedValue; // To detect redundant calls

		protected virtual void Dispose(bool disposing)
		{
			if (!_disposedValue)
			{
				if (disposing)
				{
					_service.Dispose();
					_service = null;
				}

				// TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
				// TODO: set large fields to null.

				_disposedValue = true;
			}
		}

		// TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
		// ~MachineRest() {
		//   // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
		//   Dispose(false);
		// }

		// This code added to correctly implement the disposable pattern.
		public void Dispose()
		{
			// Do not change this code. Put cleanup code in Dispose(bool disposing) above.
			Dispose(true);
			// TODO: uncomment the following line if the finalizer is overridden above.
			// GC.SuppressFinalize(this);
		}
		#endregion

	}

/*
	public class LoadOptions2Controller : ApiController
	{
		// GET api/values
		public IEnumerable<LoadInfo> Get()
		{
			using (var controller = Dependency.Resolve<IItemController>())
			{
				var list = new List<LoadInfo>();
				foreach (Item item in controller.GetAll(typeof(LoadInfo)))
				{
					list.Add((LoadInfo)controller.Create(item.ItemID));
				}
				return list;
			}
		}

		// GET api/values/5
		[ResponseType(typeof(LoadInfo))]
		public IHttpActionResult Get(int id)
		{
			try
			{
				using (var controller = Dependency.Resolve<IItemController>())
				{
					object obj = controller.Create(id);
					if (obj != null || obj is LoadInfo)
					{
						return Ok((LoadInfo)obj);
					}
					return NotFound();
				}
			}
			catch (Exception e)
			{
				return BadRequest(e.Message);
			}
		}

		// POST api/values
		public IHttpActionResult Post([FromBody]LoadInfo value)
		{
			if (!ModelState.IsValid || value == null)
			{
				return BadRequest(ModelState);
			}

			try
			{
				using (var controller = Dependency.Resolve<IItemController>())
				{
					int newid = controller.Add(value.SettingName,value);
					return CreatedAtRoute("DefaultApi", new
					{
						id = newid
					}, value);
				}
			}
			catch (Exception ex)
			{
				return BadRequest(ex.Message);
			}
		}
		// PUT api/values/5
		public IHttpActionResult Put(int id, [FromBody]LoadInfo value)
		{
			if (!ModelState.IsValid || value == null)
			{
				return BadRequest(ModelState);
			}

			try
			{
				using (var controller = Dependency.Resolve<IItemController>())
				{
					controller.Save(id,value.SettingName,value);
					return CreatedAtRoute("DefaultApi", new { id = id }, value);
				}
			}
			catch (Exception ex)
			{
				return BadRequest(ex.Message);
			}
		}

		// DELETE api/values/5
		[ResponseType(typeof(Machine))]
		public IHttpActionResult Delete(int id)
		{
			using (var controller = Dependency.Resolve<IItemController>())
			{
				var item = controller.Get(id);
				if (item == null)
				{
					return NotFound();
				}
				controller.Delete(id);

				return Ok(item);
			}
		}
	}
	*/
}
