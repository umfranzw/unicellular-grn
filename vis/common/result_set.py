class ResultSet(list):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def fetchone(self):
        return self.__getitem__(0) if self.__len__() > 0 else None
