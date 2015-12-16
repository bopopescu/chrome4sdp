'''
Copyright (c) 2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
'''

import sys,os,hashlib


def prompt(path):
  fname = os.path.join(path, 'EULA.txt')
  BLOCKSIZE = 65536
  hasher = hashlib.md5()
  try:
    f = open(fname, 'r')
    rbuf = f.read(BLOCKSIZE)
    while len(rbuf) > 0:
      hasher.update(rbuf)
      rbuf = f.read(BLOCKSIZE)
  except Exception:
    sys.exit(1)
  else:
    f.close()
  finally:
    pass

  accepted = os.path.join(os.path.expanduser('~'), '.sweEULA-' + hasher.hexdigest() + '.txt')

  if os.path.exists(accepted):
    return True
  user_pref = True
  lineCount = 20
  count = 0
  try:
    f = open(fname, 'r')
    for line in f:
      if count < lineCount:
        print line
        count = count + 1
      else:
        count = 0
        user_pref = promptUser('\n\nhit "y" to continue or "n" to quit [y/n]:')
        if user_pref == False:
          break
  except IOError:
    sys.exit(1)
  except Exception:
    sys.exit(1)
  else:
    f.close()
  finally:
    pass

  if user_pref == True:
    user_pref = promptUser('\n\nDo you Accept the EULA [y/n]: ')
  if user_pref == True:
    #user accepted the EULA. Record it so he is not
    #bothered repeatedly
    with open (accepted, 'w') as fw:
      fw.write('accepted\n')
    return True
  else:
    sys.exit(1)


def promptUser(prompt):
  print(prompt)
  sys.stdout.flush()
  return raw_input('') in ['y','Y','yes','YES','Yes']

if __name__ == '__main__':
  path = os.getcwd()
  choice = prompt(path)
  if choice:
    print 'user accepted\n'
