import getpass

def get_user():
    try:
        username = getpass.getuser()
    except OSError:
        username = 'pi'
    return username