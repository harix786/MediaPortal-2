﻿using System;
using System.Collections.Generic;
using System.Linq;
using HttpServer;
using HttpServer.Exceptions;
using MediaPortal.Common;
using MediaPortal.Common.Logging;
using MediaPortal.Common.MediaManagement;
using MediaPortal.Common.MediaManagement.DefaultItemAspects;
using MediaPortal.Common.ResourceAccess;
using MediaPortal.Plugins.MP2Extended.MAS.General;
using MediaPortal.Plugins.MP2Extended.ResourceAccess.WSS.stream;
using MediaPortal.Plugins.MP2Extended.TAS.Tv;

namespace MediaPortal.Plugins.MP2Extended.ResourceAccess.MAS.General
{
  internal class GetFileInfo : SendDataBase, IRequestMicroModuleHandler
  {
    public dynamic Process(IHttpRequest request)
    {
      HttpParam httpParam = request.Param;
      string id = httpParam["id"].Value;
      if (id == null)
        throw new BadRequestException("GetFileInfo: no id is null");

      ISet<Guid> necessaryMIATypes = new HashSet<Guid>();
      necessaryMIATypes.Add(MediaAspect.ASPECT_ID);
      necessaryMIATypes.Add(ProviderResourceAspect.ASPECT_ID);
      necessaryMIATypes.Add(ImporterAspect.ASPECT_ID);

      MediaItem item = GetMediaItems.GetMediaItemById(id, necessaryMIATypes);

      if (item == null)
        throw new BadRequestException("GetFileInfo: no media tiem found");

      SingleMediaItemAspect providerResourceAspect = MediaItemAspect.GetAspect(item.Aspects, ProviderResourceAspect.Metadata);
      var resourcePathStr = providerResourceAspect.GetAttributeValue(ProviderResourceAspect.ATTR_RESOURCE_ACCESSOR_PATH);
      var resourcePath = ResourcePath.Deserialize(resourcePathStr.ToString());

      var ra = GetResourceAccessor(resourcePath);
      IFileSystemResourceAccessor fsra = ra as IFileSystemResourceAccessor;
      if (fsra == null)
        throw new InternalServerException("GetFileInfo: failed to create IFileSystemResourceAccessor");

      WebFileInfo webFileInfo = new WebFileInfo
      {
        Exists = fsra.Exists,
        Extension = fsra.Path.Split('.').Last(),
        IsLocalFile = true,
        IsReadOnly = true,
        LastAccessTime = DateTime.Now,
        LastModifiedTime = fsra.LastChanged,
        Name = fsra.ResourceName,
        OnNetworkDrive = false,
        Path = item.MediaItemId.ToString(),
        Size = fsra.Size,
        PID = 0
      };


      return webFileInfo;
    }

    internal static ILogger Logger
    {
      get { return ServiceRegistration.Get<ILogger>(); }
    }
  }
}