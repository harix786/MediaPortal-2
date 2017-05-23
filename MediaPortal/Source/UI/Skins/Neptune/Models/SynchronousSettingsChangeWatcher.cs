#region Copyright (C) 2007-2017 Team MediaPortal

/*
    Copyright (C) 2007-2017 Team MediaPortal
    http://www.team-mediaportal.com

    This file is part of MediaPortal 2

    MediaPortal 2 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MediaPortal 2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MediaPortal 2. If not, see <http://www.gnu.org/licenses/>.
*/

#endregion

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MediaPortal.Common.Messaging;
using MediaPortal.Common.Services.Settings;

namespace MediaPortal.UiComponents.Neptune.Models
{
  public class SynchronousSettingsChangeWatcher<T>: SettingsChangeWatcher<T> where T : class
  {
    public SynchronousSettingsChangeWatcher()
    {
      if (_messageQueue != null)
      {
        _messageQueue.Shutdown();
      }
      _messageQueue = new AsynchronousMessageQueue(this, new string[] { SettingsManagerMessaging.CHANNEL });
      _messageQueue.PreviewMessage += OnMessageReceived;
      _messageQueue.Start();
    }
  }
}
