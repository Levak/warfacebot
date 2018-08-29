import discord
import subprocess
import traceback
import os
import urllib
import requests
import requests_cache
from xml.dom import minidom
from datetime import datetime, timedelta
from time import sleep
from pyquery import PyQuery

client = discord.Client()
CURRENT_DIR = os.path.dirname(os.path.realpath(__file__))
CHANNEL_ID = "484005319941423107"
BOT_NICKNAME = "PaoDoceBOT"
TOKEN = os.environ['bot_token']

def init():
    removeLockFile()
    isBotRunning()
    requests_cache.install_cache('crown_ranking_cache',expire_after=3600)
    client.run(TOKEN)

def touch(path):
  with open(path, 'a') as f:
    os.utime(path, None)
    f.close()

def isBotTsValid():
  try:
    file_mod_time = datetime.fromtimestamp(os.stat(CURRENT_DIR + "/data/.ts").st_mtime)
    now = datetime.today()
    max_delay = timedelta(seconds=30)
    return not now-file_mod_time > max_delay
  except:
    return False

def createLockFile():
  try:
    touch(CURRENT_DIR + "/wb_lock")
  except:
    return

def removeLockFile():
  try:
    os.remove(CURRENT_DIR + "/wb_lock")
  except:
    return

def isBotRunning():
  running = isBotTsValid() 
  if running:
    return True
  else:
    if not os.path.exists(CURRENT_DIR + "/wb_lock"):
      createLockFile()
      os.system("killall -9 wb &")
      os.system("killall -9 wb.sh &")
      os.system("screen -dm " + CURRENT_DIR + "/wb.sh br")
      sleep(5)
      removeLockFile()
    else:
      sleep(5)
    return isBotTsValid()

def formatNumber(num):
  return '{:20,d}'.format(num).replace(",", ".").strip()

def getClanmateRole(role):
  if role == 2:
    return "Oficial"
  elif role == 1:
    return "Líder"
  else:
    return "Membro"

def getClanmateStatus(status,nickname):
  if status == 0:
    return "Offline"
  elif status == 9:
    return "Online (no lobby)"
  elif status == 13:
    return "Online (inativo no lobby)"
  else:
    additional_status = getClanmateAdditionalStatus(nickname)
    if additional_status is False:
        return "Online"
    else:
        return additional_status

def getClanmateRanking(nickname):
  try:
    xml = minidom.parse(CURRENT_DIR + '/data/clan_list.xml')
    clan = xml.getElementsByTagName('clan')[0]
    members = xml.getElementsByTagName('clan_member_info')
    members.sort(key = lambda x: int(x.attributes['clan_points'].value))
    i = 0
    for member in reversed(members):
      i += 1
      if member.attributes['nickname'].value == nickname:
        return i
    return False
  except:
    traceback.print_exc()
    return False

def getClanmateAdditionalStatus(nickname):
  # TODO add clanwar and tutorial
  other_places = {"@ui_playerinfo_pvemission":"Jogando COOP","@ui_playerinfo_pvpmission":"Jogando PvP","@ui_playerinfo_pveroom":"Em sala de jogo COOP","@ui_playerinfo_pvproom":"Em sala de jogo PvP","@ui_playerinfo_ratingmission":"Jogando Classificatória PvP","@ui_playerinfo_inlobby":"Online (no lobby)","@ui_playerinfo_intutorial":"Jogando Tutorial"}
  pve_maps = {"@zombietowereasy":"Black Shark Fácil","@zombietowernormal":"Black Shark Normal","@zombietowerhard":"Black Shark Pro","@icebreakereasy":"Icebreaker Fácil","@icebreakernormal":"Icebreaker Normal","@icebreakerhard":"Icebreaker Pro","@anubiseasy2":"Escape From Anubis Fácil","@anubisnormal2":"Escape From Anubis Normal","@anubishard2":"Escape From Anubis Pro","@anubiseasy":"Anubis Fácil","@anubisnormal":"Anubis Normal","@anubishard":"Anubis Pro","@volcanoeasy":"Volcano Fácil","@volcanonormal":"Volcano Normal","@volcanohard":"Volcano Pro","@volcanosurvival":"Volcano Hardcore","@zombieeasy":"Darkness Fácil","@zombienormal":"Darkness Normal","@zombiehard":"Darkness Pro","@easymission":"Fácil","@normalmission":"Normal","@hardmission":"Pro","@campaignsection1":"Ponta de lança","@campaignsection2":"Emboscada","@campaignsection3":"Zênite","@campaignsections":"Maratona","@chernobyleasy":"Chernobyl Fácil","@chernobylnormal":"Chernobyl Normal","@chernobylhard":"Chernobyl Pro","@survivalmission":"Assassinato","@trainingmission":"Introdução"}
  pvp_maps = {"@pvp_mission_display_name_ctf_breach":"Capture/Breach","@pvp_mission_display_name_ctf_construction":"Capture/Construction","@pvp_mission_display_name_ctf_convoy":"Capture/Convoy","@pvp_mission_display_name_ctf_deposit":"Capture/Deposit","@pvp_mission_display_name_ctf_longway":"Capture/Longway","@pvp_mission_display_name_ctf_quarry":"Capture/Quarry","@pvp_mission_display_name_ctf_test":"Capture","@pvp_mission_display_name_ctf_vault_fwc":"Capture/Vault 14","@pvp_mission_display_name_ctf_vault":"Capture/Vault","@pvp_mission_display_name_dmn_armageddon":"Domination/Armageddon","@pvp_mission_display_name_dmn_downtown":"Domination/Downton","@pvp_mission_display_name_dmn_sirius":"Domination/Sirius","@pvp_mission_display_name_dmn_subzero":"Domination/Subzero","@pvp_mission_display_name_dst_afghan":"Destruction/Towers","@pvp_mission_display_name_dst_lighthouse":"Destruction/Lighthouse","@pvp_mission_display_name_ffa_bunker":"Free For All/Bunker","@pvp_mission_display_name_ffa_downtown":"Free For All/Downtown","@pvp_mission_display_name_ffa_forest_dawn":"Free For All/Forest","@pvp_mission_display_name_ffa_forest":"Free For All/Forest","@pvp_mission_display_name_ffa_motel":"Free For All/Motel","@pvp_mission_display_name_ffa_overpass":"Free For All/Overpass","@pvp_mission_display_name_ffa_train":"Free For All/Train","@pvp_mission_display_name_ffa_widestreet":"Free For All/Widestreet","@pvp_mission_display_name_hnt_africa":"Hunt/Africa","@pvp_mission_display_name_hnt_night_motel":"Hunt/Mojave","@pvp_mission_display_name_hnt_winter":"Hunt/Siberia","@pvp_mission_display_name_hnt_winter_xmas":"Hunt/Sibéria Natalina","@pvp_mission_display_name_ptb_afghan":"Plant The Bomb/Yard","@pvp_mission_display_name_ptb_afghan_up":"Plant The Bomb/Yard 2.0","@pvp_mission_display_name_ptb_bridges":"Plant The Bomb/Bridges","@pvp_mission_display_name_ptb_bridges_up":"Plant The Bomb/Bridges 2.0","@pvp_mission_display_name_ptb_d17":"Plant The Bomb/D17","@pvp_mission_display_name_ptb_destination":"Plant The Bomb/Destination","@pvp_mission_display_name_ptb_district":"Plant The Bomb/District","@pvp_mission_display_name_ptb_factory":"Plant The Bomb/Factory","@pvp_mission_display_name_ptb_factory_up":"Plant The Bomb/Factory 2.0","@pvp_mission_display_name_ptb_mine":"Plant The Bomb/Quarry","@pvp_mission_display_name_ptb_overpass":"Plant The Bomb/Overpass","@pvp_mission_display_name_ptb_palace":"Plant The Bomb/Palace","@pvp_mission_display_name_ptb_pyramid":"Plant The Bomb/Pyramid","@pvp_mission_display_name_ptb_trailerpark":"Plant The Bomb/Trailer Park","@pvp_mission_display_name_stm_blackgold":"Storm/Blackgold","@pvp_mission_display_name_stm_blackmamba":"Storm/Black Mamba","@pvp_mission_display_name_stm_highhill":"Storm/Highill","@pvp_mission_display_name_stm_invasion":"Storm/Invasion","@pvp_mission_display_name_stm_wharf":"Storm/Wharf","@pvp_mission_display_name_tdm_airbase":"Team Deathmatch/Airbase","@pvp_mission_display_name_tdm_aul_pts":"Team Deathmatch/Aul","@pvp_mission_display_name_tdm_aul":"Team Deathmatch/Aul","@pvp_mission_display_name_tdm_codenamed18":"Team Deathmatch/Codename D18","@pvp_mission_display_name_tdm_crossriver":"Team Deathmatch/Crossriver","@pvp_mission_display_name_tdm_dock":"Team Deathmatch/Dock","@pvp_mission_display_name_tdm_downtown":"Team Deathmatch/Downtown","@pvp_mission_display_name_tdm_farm_hw":"Team Deathmatch/Farm Halloween","@pvp_mission_display_name_tdm_farm_sunset":"Team Deathmatch/Farm Sunset","@pvp_mission_display_name_tdm_farm":"Team Deathmatch/Farm","@pvp_mission_display_name_tdm_grand_bazaar":"Team Deathmatch/Grand Bazaar","@pvp_mission_display_name_tdm_ghost_town":"Team Deathmatch/Ghost Town","@pvp_mission_display_name_tdm_hangar_up":"Team Deathmatch/Hangar 2.0","@pvp_mission_display_name_tdm_hangar_og16":"Team Deathmatch/Hangar Og16","@pvp_mission_display_name_tdm_hangar_xmas":"Team Deathmatch/Xmas Hangar","@pvp_mission_display_name_tdm_hangar":"Team Deathmatch/Hangar","@pvp_mission_display_name_tdm_motel":"Team Deathmatch/Motel","@pvp_mission_display_name_tdm_oildepot_e16":"Team Deathmatch/Oil Depot Eu2016","@pvp_mission_display_name_tdm_oildepot_wc18":"Team Deathmatch/Oil Depot Mundial 2018","@pvp_mission_display_name_tdm_oildepotv3":"Team Deathmatch/Oil Depot","@pvp_mission_display_name_tdm_oildepot_xmas":"Team Deathmatch/Oil Depot Xmas","@pvp_mission_display_name_tdm_residential":"Team Deathmatch/Trailer Park","@pvp_mission_display_name_tdm_shuttle":"Team Deathmatch/Shuttle","@pvp_mission_display_name_tdm_sirius":"Team Deathmatch/Sirius","@pvp_mission_display_name_tdm_streetwars":"Team Deathmatch/Street Wars","@pvp_mission_display_name_tbs_hawkrock":"Blitz/Hawkrock","@pvp_mission_display_name_tbs_waterfalling":"Blitz/Waterfalling","@pvp_mission_display_name_tbs_waterfalling_xmas":"Blitz/Waterfalling Xmas","@pvp_mission_display_name_lms_mojave":"Battle Royale/Mojave","@pvp_mission_display_name_lms_pripyat":"Battle Royale/Pripyat"}
  try:
    xml = minidom.parse(CURRENT_DIR + '/data/clanmates/' + nickname + '.xml')
    info = xml.getElementsByTagName('peer_clan_member_update')[0]
    place_info_token = info.attributes['place_info_token'].value
    mission_info_token = info.attributes['mission_info_token'].value
    place_token = info.attributes['place_token'].value
    if place_token in other_places:
        result = other_places[place_token]
        if place_info_token in pve_maps:
            result += "/" + pve_maps[place_info_token]
        elif mission_info_token in pvp_maps:
            result += "/" + pvp_maps[mission_info_token]
        return result
    else:
        return False
  except:
    traceback.print_exc()
    return False

def getRankingNumberByExp(exp):
  result = {"number":"","name":""}
  exps = (0,120,620,1500,2500,5800,8100,11100,14600,18800,23800,29600,36300,44100,53000,63000,74500,87400,102000,118400,136700,157200,180000,205200,233300,264400,298700,336500,378000,423700,473700,528400,588200,653400,724400,801600,885500,976400,1074800,1181100,1296000,1419700,1552900,1696200,1849900,2014800,2191200,2380000,2581500,2796400,3025300,3268800,3527500,3801900,4092800,4400600,4726000,5069500,5431800,6000000,6568200,7136400,7704600,8272800,8841000,9409200,9977400,10545600,11113800,11682000,12250200,12818400,13386600,13954800,14523000,15091200,15659400,16227600,16795800,17364000,17948250,18473250,19068600,19636800,20123250,20773200,21341400,21909600,22477800,23046000)
  names = ("Trainee","Cadete Júnior","Cadete","Cadete Sênior","Cadete de 1ª Classe","Recruta","Soldado","Soldado Raso de 2ª Classe","Soldado Raso","Soldado Raso de 1ª Classe","Especialista","Especialista de Artilharia","Especialista Técnico","Especialista de 1ª Classe","Patrulheiro","Soldado EP","Cabo","Líder de Tiro","Sargento de 3ª Classe","Sargento de 2ª Classe","Sargento","Sargento de 1ª Classe","Sargento de Apoio","Sargento de Artilharia","Sargento Mestre","Primeiro Sargento","Sargento Comandante","Sargento de Artilharia Mestre","Sargento Maior","Sargento de Companhia","Candidato a Sargento Oficial","Sargento Oficial","Sargento Oficial Chefe","Sargento Oficial Chefe de 1ª Classe","Sargento Oficial Mestre","Intendente","Cadete-Oficial Júnior","Cadete-Oficial Sênior","Aspirante","Segundo Tenente","Primeiro Tenente","Subtenente","Tenente","Tenente-Coronel","Tenente-Capitão","Capitão","Capitão 1","Capitão 2","Capitão 3","Coronel","Brigadeiro","Marechal de Campo","Comandante","Alto-Comandante","Supremo Comandante","Major-General","Tenente-General","General-Marechal de Campo","Segundo Tenente SF","Primeiro Tenente SF","Subtenente SF","Tenente SF","Tenente-Coronel SF","Tenente-Capitão SF","Capitão SF","Capitão 1 SF","Capitão 2 SF","Capitão 3 SF","Coronel SF","Warface","Morrigan","Odin","Mars","Athena","Ashur","Indra","Guan Yu","Mantus","Artaius","Ares","Apache","Spartan","Maori","Vityaz","Hussar","Viking","Shinobi","Mogul","Templar","Samurai")
  i=0
  for exp_ in exps:
    if exp >= exp_:
      i += 1
  result["number"] = i;
  result["name"] = names[i-1];
  return result

def getLastSeen(profile_id):
  try:
    return open(CURRENT_DIR + "/data/clanmates/lastseen/" + profile_id,"r").read().strip()
  except:
    traceback.print_exc()
    return False

def getClanCrownRanking(clan):
  try:
    url = "http://warface.uol.com.br/ranking-de-clas?" + urllib.parse.urlencode({"clanName":clan})
    html = requests.get(url).text
    pq = PyQuery(html)
    tag = pq('#clans table tr')
    for tr in tag.items():
      td = tr.find("td")
      clan = {"pos":"","name":"","members":"","pc":""}
      clan["pos"] = td.eq(0).text()
      clan["name"] = td.eq(1).text()
      clan["members"] = td.eq(2).text()
      clan["pc"] = td.eq(3).text().replace(" ",".")
      return clan
    return False
  except:
    return False

async def getGlobalCrownRanking():
  try:
    url = "http://warface.uol.com.br/ranking-de-clas/1.lhtml"
    html = requests.get(url).text
    clans = []
    pq = PyQuery(html)
    tag = pq('#clans table tr')
    i = 0
    for tr in tag.items():
      i += 1
      td = tr.find("td")
      clan = {"pos":"","name":"","members":"","pc":""}
      clan["pos"] = td.eq(0).text()
      clan["name"] = td.eq(1).text()
      clan["members"] = td.eq(2).text()
      clan["pc"] = td.eq(3).text().replace(" ",".")
      clans.append(clan)
    if i == 0:
      return False
    msg = "```"
    msg += "Ranking mensal de clãs\n\n" 
    for clan in clans:
      msg += clan["pos"] + ". " + clan["name"] + " - " + clan["members"] + " - " + clan["pc"] + " PC\n\n"
    msg += "```"
    return msg
  except:
    return False

async def getClanRanking():
  try:
    msg = "```"
    xml = minidom.parse(CURRENT_DIR + '/data/clan_list.xml')
    clan = xml.getElementsByTagName('clan')[0]
    clan_name = str(clan.attributes['name'].value)
    members = xml.getElementsByTagName('clan_member_info')
    members.sort(key = lambda x: int(x.attributes['clan_points'].value))
    crown_ranking = getClanCrownRanking(clan_name)
    msg += "\n" + clan_name + "\n\n" 
    i = 0
    for member in reversed(members):
      if member.attributes['nickname'].value.lower() != BOT_NICKNAME.lower():
        i += 1
        msg += str(i) + ". " + member.attributes['nickname'].value + " - " + formatNumber(int(member.attributes['clan_points'].value)) + " PC - " + getClanmateRole(int(member.attributes['clan_role'].value)) + "\n\n"
    if crown_ranking:
      msg += "\nPosição do clã (crown mensal): " + crown_ranking["pos"] + " - " + crown_ranking["pc"] + " PC"
    else:
      msg += "\nPosição do clã (crown mensal): Indisponível no momento"
    msg += "\nPosição do clã (geral): " + formatNumber(int(clan.attributes['leaderboard_position'].value))
    msg += "\nTotal de pontos: " + formatNumber(int(clan.attributes['clan_points'].value))
    msg += "\nTotal de membros: " + str(int(clan.attributes['members'].value) - 1)
    msg += "```"
    return msg
  except:
    traceback.print_exc()
    return False

async def getOnlineUsers():
  try:
    players = ""
    msg = "```"
    xml = minidom.parse(CURRENT_DIR + '/data/clan_list.xml')
    members = xml.getElementsByTagName('clan_member_info')
    members.sort(key = lambda x: str(x.attributes['nickname'].value))
    i=0
    for member in members:
      if int(member.attributes['status'].value) is not 0 and member.attributes['nickname'].value.lower() != BOT_NICKNAME.lower():
        i += 1
        players += member.attributes['nickname'].value + " - " + getClanmateStatus(int(member.attributes['status'].value),member.attributes['nickname'].value) + "\n\n"
    if i is 0:
      msg += "Não há jogadores online no momento"
    else:
      msg += players
      if i is 1:
        msg += "\n1 jogador online"
      else:
        msg += "\n" + str(i) + " jogadores online"
    msg += "```"
    return msg
  except:
    traceback.print_exc()
    return False

async def getUserInfo(nickname):
  try:
    msg = "```"
    xml = minidom.parse(CURRENT_DIR + '/data/clan_list.xml')
    members = xml.getElementsByTagName('clan_member_info')
    found=False
    for member in members:
      if member.attributes['nickname'].value.lower() == nickname.lower() and member.attributes['nickname'].value.lower() != BOT_NICKNAME.lower():
        found=True
        msg += "Nickname: " + member.attributes['nickname'].value + "\n\n"
        status = int(member.attributes['status'].value)
        profile_id = str(member.attributes['profile_id'].value)
        exp = int(member.attributes['experience'].value)
        ranking = getRankingNumberByExp(exp)
        clan_pos = getClanmateRanking(member.attributes['nickname'].value)
        msg += "Patente: " + ranking["name"] + " (" + str(ranking["number"]) + ") - " + formatNumber(int(member.attributes['experience'].value)) + " EXP\n\n"
        msg += "Total de PC adquirido: " + formatNumber(int(member.attributes['clan_points'].value)) + " PC\n\n"
        if clan_pos:
          msg += "Posição no clã: " + str(clan_pos) + "\n\n"
        if status != 0:
          msg += "Status: " + getClanmateStatus(status,member.attributes['nickname'].value) + "\n\n"
        else:
          lastseen = getLastSeen(profile_id)
          if lastseen:
            msg += "Status: Offline (desde " + str(lastseen) + ")" + "\n\n"
          else:
            msg += "Status: Offline\n\n"
    if found is False:
      msg += "O player '" + nickname + "' não foi encontrado"
    msg += "```"
    return msg
  except:
    traceback.print_exc()
    return False

async def showErrorMessage(message):
  await client.send_message(message.channel, '{0} {1}'.format(message.author.mention, "Não foi possível obter as informações, tente novamente mais tarde :("))

@client.event
async def on_message(message):
  if message.author == client.user or message.channel.id != CHANNEL_ID:
    return

  if message.content == '!pc' or message.content == '!ranking':
    if not isBotRunning():
      await showErrorMessage(message)
    else:
      msg = await getClanRanking()
      if msg:
        await client.send_message(message.channel, '{0} {1}'.format(message.author.mention, msg))
      else:
        await showErrorMessage(message)
  elif message.content == '!crown' or message.content == '!rankingmensal':
    if not isBotRunning():
      await showErrorMessage(message)
    else:
      msg = await getGlobalCrownRanking()
      if msg:
        await client.send_message(message.channel, '{0} {1}'.format(message.author.mention, msg))
      else:
        await showErrorMessage(message)
  elif message.content == '!online':
    if not isBotRunning():
      await showErrorMessage(message)
    else:
      msg = await getOnlineUsers()
      if msg:
        await client.send_message(message.channel, '{0} {1}'.format(message.author.mention, msg))
      else:
        await showErrorMessage(message)
  elif message.content.startswith('!info ') or message.content.startswith('!player '):
    if not isBotRunning():
      await showErrorMessage(message)
    else:
      nickname = message.content
      if nickname.startswith("!info "):
        nickname = nickname[6:]
      elif nickname.startswith("!player "):
        nickname = nickname[8:]
      msg = await getUserInfo(nickname)
      if msg:
        await client.send_message(message.channel, '{0} {1}'.format(message.author.mention, msg))
      else:
        await showErrorMessage(message)
  elif message.content == '!help':
    msg = "Comandos:\n\n"
    msg += "!pc ou !ranking - Mostra o ranking geral\n"
    msg += "!crown ou !rankingmensal - Mostra o ranking mensal/crown\n"
    msg += "!online - Mostra os usuários que estão online\n"
    msg += "!info [nickname] ou !player [nickname] - Mostra as informações do player especificado\n"
    await client.send_message(message.channel, msg)

@client.event
async def on_ready():
  print('Logged in as')
  print(client.user.name)
  print(client.user.id)
  print('------')

init()