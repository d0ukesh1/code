from KlAkOAPI.Params import KlAkParams, paramParams, KlAkArray
from KlAkOAPI.AdmServer import KlAkAdmServer
from KlAkOAPI.ReportManager import KlAkReportManager
from KlAkOAPI.HostGroup import KlAkHostGroup
from KlAkOAPI.ChunkAccessor import KlAkChunkAccessor
from KlAkOAPI.ListTags import KlAkListTags
from data_processing import *
from urllib3.exceptions import InsecureRequestWarning
from urllib3 import disable_warnings

class O_API:

    def __init__(self, server_url=None, username=None, password=None):
        self.server_url = server_url
        self.username = username
        self.password = password
        self.server = self.GetServer()
        disable_warnings(InsecureRequestWarning)

    def GetServer(self):
        """Connects to KSC server"""
        try:
            server = KlAkAdmServer.Create(self.server_url, self.username, self.password, verify=False)
            Logger.log_info("Аутентификация прошла успешно.")
            return server
        except Exception as e:
            Logger.log_error(f"Ошибка при продключении к серверу {self.server_url} : {str(e)}")
            return None

    def FindHostByDN(self,hostdn):
        """Find host by display name and get hostname"""
        try:
            if not self.server:
                raise Exception("Нет соединения с сервером.")
            Logger.log_info(f"Поиск устройства: {hostdn}")
            strAccessor = KlAkHostGroup(self.server).FindHosts('KLHST_WKS_DN=\"' + hostdn + '\"', ['KLHST_WKS_HOSTNAME', 'KLHST_WKS_DN'], [], {'KLGRP_FIND_FROM_CUR_VS_ONLY': False}, lMaxLifeTime = 60 * 60).OutPar('strAccessor')

            oChunkAccessor = KlAkChunkAccessor (self.server)
            oResult = KlAkArray([])

            oChunk = oChunkAccessor.GetItemsChunk(strAccessor)
            oHosts = oChunk.OutPar('pChunk')['KLCSP_ITERATOR_ARRAY']
            for oObj in oHosts:
                Logger.log_info('Найдено устройство: ' + oObj['KLHST_WKS_DN'])
                oResult.Add(oObj.GetValue('KLHST_WKS_HOSTNAME'))

            return oResult
        except Exception as e:
            Logger.log_error(f"Устройство {hostdn} не найдено. Ошибка: {str(e)}")

    #'KLHST_WKS_STATUS','KLHST_WKS_FQDN','KLHST_WKS_STATUS_ID','KLHST_WKS_STATUS_MASK'
    def GetHostInfo(self,hostname):
        try:
            oResult = KlAkHostGroup(self.server).GetHostInfo(hostname,['KLHST_WKS_STATUS','KLHST_WKS_FQDN','KLHST_WKS_STATUS_ID','KLHST_WKS_STATUS_MASK'])
            return oResult
        except Exception as e:
            Logger.log_error(f"Информация об устройстве не получено. Ошибка: {str(e)}")
